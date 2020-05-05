// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Worker.hpp>
#include <Net/Udp/Logger.hpp>

// Qt Header
#include <QTimer>

#include <QUdpSocket>
#include <QNetworkDatagram>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace net::udp;

// clang-format off
#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
# define LOG_DEV_DEBUG(str, ...) Logger::WORKER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  do {} while (0)
#else
# define LOG_DEV_INFO(str, ...)  Logger::WORKER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  do {} while (0)
#else
# define LOG_DEV_WARN(str, ...)  Logger::WORKER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   do {} while (0)
#else
# define LOG_DEV_ERR(str, ...)   Logger::WORKER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#define LOG_DEBUG(str, ...)      Logger::WORKER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_INFO(str, ...)       Logger::WORKER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_WARN(str, ...)       Logger::WORKER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_ERR(str, ...)        Logger::WORKER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__)
// clang-format on

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

Worker::Worker(QObject* parent) : QObject(parent) { LOG_DEV_DEBUG("Constructor"); }

Worker::~Worker() { LOG_DEV_DEBUG("Destructor"); }

bool Worker::isBounded() const { return _isBounded; }

quint64 Worker::watchdogTimeout() const { return _watchdogTimeout; }

QString Worker::rxAddress() const { return _rxAddress; }

quint16 Worker::rxPort() const { return _rxPort; }

quint16 Worker::txPort() const { return _txPort; }

QMap<QString, bool> Worker::multicastGroups() const { return _multicastGroups; }

QNetworkInterface Worker::multicastInterface() const { return _multicastInterface; }

bool Worker::multicastLoopback() const { return _multicastLoopback; }

quint8 Worker::multicastTtl() const { return _multicastTtl; }

bool Worker::inputEnabled() const { return _inputEnabled; }

bool Worker::separateRxTxSocketsChanged() const { return _separateRxTxSockets; }

QUdpSocket* Worker::rxSocket() const
{
    if(_separateRxTxSockets)
        return _rxSocket.get();
    return _socket.get();
}

size_t Worker::cacheSize() const { return _cache.size(); }

bool Worker::resizeCache(size_t length) { return _cache.resize(length); }

void Worker::clearCache() { _cache.clear(); }

void Worker::releaseCache() { _cache.release(); }

std::shared_ptr<Datagram> Worker::makeDatagram(const size_t length)
{
    return _cache.make(length);
}

void Worker::onRestart()
{
    onStop();
    onStart();
}

void Worker::onStart()
{
    const bool useTwoSockets = (_separateRxTxSockets || _txPort) && _inputEnabled;
    if(_socket)
    {
        LOG_DEV_ERR("Can't start udp socket worker because socket is already valid");
        return;
    }

    LOG_DEV_INFO("Start Udp Socket Worker rx : {}:{}, tx port : {}, ", _rxAddress.toStdString(),
        _rxPort, _txPort);

    _isBounded = false;
    _watchdog = nullptr;

    // ) Create the socket
    _socket = std::make_unique<QUdpSocket>(this);
    if(useTwoSockets)
    {
        LOG_DEV_INFO("Create separate rx socket");
        _rxSocket = std::make_unique<QUdpSocket>(this);
    }

    connect(
        this, &Worker::queueStartWatchdog, this,
        [this]()
        {
            if(!_watchdog)
            {
                onStop();

                // ) Create a watchdog timer
                _watchdog = std::make_unique<QTimer>();
                _watchdog->setTimerType(Qt::TimerType::VeryCoarseTimer);

                // ) Connect timeout
                connect(
                    _watchdog.get(), &QTimer::timeout, this, [this]() { onRestart(); },
                    Qt::ConnectionType::QueuedConnection);

                LOG_INFO("Start watchdog to restart socket in {} millis", watchdogTimeout());
                // Start the watchdog
                _watchdog->start(watchdogTimeout());
            }
            else
                LOG_DEV_WARN("Watchdog already running, fail to start it");
        },
        Qt::QueuedConnection);

    // ) Connect to socket signals
    connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        this, &Worker::onSocketError);
    if(rxSocket())
    {
        connect(rxSocket(), &QUdpSocket::readyRead, this, &Worker::readPendingDatagrams);
        // _socket is always bounded because binded to QHostAddress(). Only rxSocket can go wrong
        connect(rxSocket(), &QUdpSocket::stateChanged, this, &Worker::onSocketStateChanged);
    }

    if(useTwoSockets)
    {
        connect(_rxSocket.get(),
            QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
            &Worker::onRxSocketError);
    }

    // ) Bind to socket
    // When only in output mode, calling _socket->bind() doesn't allow to set multicast ttl.
    // But calling _socket->bind(QHostAddress()) allow to set the ttl.
    // From what i understand, _socket->bind() will call _socket->bind(QHostAddress::Any) internally.
    // _socket->bind(QHostAddress()) bind to a non valid host address and random port will be choose.
    // Qt multicast issues are non resolved ? https://forum.qt.io/topic/78090/multicast-issue-possible-bug/17
    const auto bindSuccess = [&]()
    {
        if(useTwoSockets)
            return _rxSocket->bind(QHostAddress(_rxAddress), _rxPort,
                       QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint) &&
                   _socket->bind(QHostAddress(), _txPort,
                       QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

        if(_inputEnabled)
            return _socket->bind(QHostAddress(_rxAddress), _rxPort,
                QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
        return _socket->bind(QHostAddress(), _txPort,
            QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    }();

    if(bindSuccess)
    {
        LOG_INFO("Success bind to {}: {}", qPrintable(_socket->localAddress().toString()),
            _socket->localPort());

        setMulticastInterfaceNameToSocket();
        setMulticastLoopbackToSocket();
        if(_inputEnabled && rxSocket())
        {
            for(auto it = _multicastGroups.begin(); it != _multicastGroups.end(); ++it)
            {
                it.value() = _multicastInterface.isValid() ?
                                 rxSocket()->joinMulticastGroup(
                                     QHostAddress(it.key()), _multicastInterface) :
                                 rxSocket()->joinMulticastGroup(QHostAddress(it.key()));
                if(it.value())
                    LOG_INFO("Success join multicast group {}", qPrintable(it.key()));
                else
                    LOG_ERR("Fail to join multicast group {}", qPrintable(it.key()));
            }
        }

        startBytesCounter();
    }
    else
    {
        LOG_ERR("Fail to bind to {} : {}", qPrintable(_rxAddress.isEmpty() ? "Any" : _rxAddress),
            _rxPort);
        _socket = nullptr;
        _rxSocket = nullptr;
        startWatchdog();
    }
}

void Worker::onStop()
{
    if(!_socket)
    {
        LOG_DEV_WARN("Can't stop udp socket worker because socket isn't valid");
        return;
    }

    LOG_INFO("Stop Udp Socket Worker");

    stopBytesCounter();
    stopWatchdog();
    disconnect(this, nullptr, this, nullptr);

    _socket = nullptr;
    _rxSocket = nullptr;
    _multicastTtl = 0;

    for(auto& it: _multicastGroups) it = false;
}

void Worker::setWatchdogTimeout(const quint64 ms)
{
    if(_watchdogTimeout != ms)
        _watchdogTimeout = ms;
}

void Worker::setAddress(const QString& address)
{
    if(address != _rxAddress)
    {
        _rxAddress = address;
        onRestart();
    }
}

void Worker::setRxPort(const quint16 port)
{
    if(port != _rxPort)
    {
        _rxPort = port;
        if(_inputEnabled)
            onRestart();
    }
}

void Worker::setTxPort(const quint16 port)
{
    if(port != _txPort)
    {
        _txPort = port;
        onRestart();
    }
}

void Worker::joinMulticastGroup(const QString& address)
{
    LOG_INFO("Join Multicast group {}", address.toStdString());

    if(!_inputEnabled)
        return;

    const auto hostAddress = QHostAddress(address);
    if(!address.isEmpty() && hostAddress.isMulticast())
    {
        if(!_multicastGroups.contains(address))
        {
            const bool successJoin =
                rxSocket() &&
                (_multicastInterface.isValid() ?
                        rxSocket()->joinMulticastGroup(hostAddress, _multicastInterface) :
                        rxSocket()->joinMulticastGroup(hostAddress));
            _multicastGroups.insert(address, successJoin);
        }
    }
}

void Worker::leaveMulticastGroup(const QString& address)
{
    LOG_INFO("Leave Multicast group {}", address.toStdString());

    if(_multicastGroups.contains(address))
    {
        const bool successLeave =
            rxSocket() &&
            (_multicastInterface.isValid() ?
                    rxSocket()->leaveMulticastGroup(QHostAddress(address), _multicastInterface) :
                    rxSocket()->leaveMulticastGroup(QHostAddress(address)));
        _multicastGroups.remove(address);
    }
}

void Worker::setMulticastInterfaceName(const QString& name)
{
    LOG_INFO("Set Multicast Interface Name : {}", name.toStdString());

    if(name != _multicastInterface.name())
    {
        const auto iface = QNetworkInterface::interfaceFromName(name);
        _multicastInterface = iface;
        onRestart();
    }
}

void Worker::setMulticastLoopback(const bool loopback)
{
    LOG_DEV_INFO("Set Multicast Loopback : {}", loopback);

    if(_multicastLoopback != loopback)
    {
        _multicastLoopback = loopback;
        setMulticastLoopbackToSocket();
    }
}

void Worker::setInputEnabled(const bool enabled)
{
    LOG_DEV_INFO("Set Input Enabled : {}", enabled);
    if(enabled != _inputEnabled)
    {
        _inputEnabled = enabled;
        onRestart();
    }
}

void Worker::setSeparateRxTxSockets(const bool separateRxTxSocketsChanged)
{
    const bool shouldUseSeparate = separateRxTxSocketsChanged || _txPort;
    if(shouldUseSeparate != _separateRxTxSockets)
    {
        _separateRxTxSockets = shouldUseSeparate;
        if(_inputEnabled)
            onRestart();
    }
}

void Worker::setMulticastInterfaceNameToSocket() const
{
    if(_socket && _multicastInterface.isValid())
    {
        LOG_INFO("Set outgoing interface {} for multicast packets",
            _multicastInterface.name().toStdString());
        _socket->setMulticastInterface(_multicastInterface);
        const auto i = _socket->multicastInterface();
        if(!i.isValid())
            LOG_ERR("Can't use {} as output multicast interface",
                _multicastInterface.name().toStdString());
    }
}

void Worker::setMulticastLoopbackToSocket() const
{
    if(rxSocket() && _inputEnabled)
    {
        LOG_INFO("Set MulticastLoopbackOption to {}", int(_multicastLoopback));
        rxSocket()->setSocketOption(
            QAbstractSocket::SocketOption::MulticastLoopbackOption, _multicastLoopback);
    }
}

void Worker::startWatchdog() { Q_EMIT queueStartWatchdog(); }

void Worker::stopWatchdog() { _watchdog = nullptr; }

void Worker::setMulticastTtl(const quint8 ttl)
{
    if(!_socket)
        return;

    if(!ttl)
        return;

    if(ttl != _multicastTtl)
    {
        _socket->setSocketOption(QAbstractSocket::MulticastTtlOption, int(ttl));
        _multicastTtl = ttl;
    }
}

void Worker::onSendDatagram(const SharedDatagram& datagram)
{
    if(!isBounded())
    {
        LOG_DEV_ERR("Can't send datagram if socket isn't bounded");
        return;
    }
    if(!datagram)
    {
        LOG_DEV_ERR("Can't send null datagram");
        return;
    }

    if(!_socket)
    {
        LOG_DEV_ERR("Can't send a datagram when the socket is null");
        return;
    }

    if(datagram->destinationAddress.isNull())
    {
        LOG_DEV_ERR("Can't send datagram to null address");
        return;
    }

    if(!datagram->buffer())
    {
        LOG_DEV_ERR("Can't send datagram with empty buffer");
        return;
    }

    if(!datagram->length())
    {
        LOG_DEV_ERR("Can't send datagram with data length to 0");
        return;
    }

    const auto bytesWritten = [&]()
    {
        const QHostAddress host(datagram->destinationAddress);
        const bool isMulticast = host.isMulticast();

        // Can't set ttl with qt api by passing a const char* buffer, we need to copy to QByteArray
        if(datagram->ttl && !isMulticast)
        {
            // Copy will happen :(
            // Don't have other choice in order to set ttl
            QNetworkDatagram d(QByteArray(reinterpret_cast<const char*>(datagram->buffer()),
                                   int(datagram->length())),
                host, datagram->destinationPort);
            d.setHopLimit(datagram->ttl);
            return _socket->writeDatagram(d);
        }

        if(isMulticast)
            setMulticastTtl(datagram->ttl);
        return _socket->writeDatagram(reinterpret_cast<const char*>(datagram->buffer()),
            datagram->length(), host, datagram->destinationPort);
    }();

    if(bytesWritten <= 0 || bytesWritten != datagram->length())
    {
        startWatchdog();

        if(bytesWritten <= 0)
            LOG_ERR("Fail to send datagram, 0 bytes written. Restart Socket. {}",
                _socket->errorString().toStdString());
        else
            LOG_ERR("Fail to send datagram, {}/{} bytes written. Restart Socket. {}",
                static_cast<long long>(bytesWritten), static_cast<long long>(datagram->length()),
                _socket->errorString().toStdString());
        return;
    }

    _txBytesCounter += bytesWritten;
    ++_txPacketsCounter;
}

bool Worker::isPacketValid(const uint8_t* buffer, const size_t length) const
{
    return buffer && length;
}

void Worker::readPendingDatagrams()
{
    if(!rxSocket())
        return;

    while(rxSocket() && rxSocket()->isValid() && rxSocket()->hasPendingDatagrams())
    {
        if(rxSocket()->pendingDatagramSize() == 0)
        {
            LOG_DEV_WARN(
                "Receive datagram with size 0. This may means : \n"
                "- That host is unreachable (receive an ICMP packet destination unreachable).\n"
                "- Your OS doesn't support IGMP (if last packet sent was multicast). "
                "On unix system you can check with netstat -g");
            ++_rxInvalidPacket;
            // This might happen, so don't close socket.
            // This will cause an error  Connection reset by peer, that we need to ignore
            // If we don't read, then we won't receive data anymore
            rxSocket()->receiveDatagram(0);
            return;
        }

        QNetworkDatagram datagram = rxSocket()->receiveDatagram();

        if(!datagram.isValid())
        {
            LOG_ERR("Receive datagram that is marked not valid. Restart Socket."
                    "This may be a sign that your OS doesn't support IGMP. On unix system you can "
                    "check with netstat -g");
            ++_rxInvalidPacket;
            startWatchdog();
            return;
        }

        if(datagram.data().size() <= 0)
        {
            LOG_ERR("Receive datagram with size {}. Restart Socket.", datagram.data().size());
            ++_rxInvalidPacket;
            startWatchdog();
            return;
        }

        if(!isPacketValid(reinterpret_cast<const uint8_t*>(datagram.data().constData()),
               datagram.data().size()))
        {
            LOG_WARN("Receive not valid application datagram. Simply discard the packet");
            ++_rxInvalidPacket;
            continue;
        }

        if(datagram.data().size() > 65535)
        {
            LOG_ERR("Receive a datagram with size of {}, that is too big for a datagram. Restart "
                    "Socket.",
                datagram.data().size());
            ++_rxInvalidPacket;
            startWatchdog();
            return;
        }

        SharedDatagram sharedDatagram = makeDatagram(datagram.data().size());
        memcpy(sharedDatagram.get()->buffer(),
            reinterpret_cast<const uint8_t*>(datagram.data().constData()), datagram.data().size());
        sharedDatagram->destinationAddress = datagram.destinationAddress().toString();
        sharedDatagram->destinationPort = datagram.destinationPort();
        sharedDatagram->senderAddress = datagram.senderAddress().toString();
        sharedDatagram->senderPort = datagram.senderPort();
        sharedDatagram->ttl = datagram.hopLimit();

        _rxBytesCounter += datagram.data().size();
        ++_rxPacketsCounter;

        onDatagramReceived(sharedDatagram);
    }
}

void Worker::onDatagramReceived(const SharedDatagram& datagram)
{
    Q_EMIT datagramReceived(datagram);
}

void Worker::onSocketError(QAbstractSocket::SocketError error)
{
    onSocketErrorCommon(error, _socket.get());
}

void Worker::onRxSocketError(QAbstractSocket::SocketError error)
{
    onSocketErrorCommon(error, _rxSocket.get());
}

void Worker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    LOG_INFO("Socket State Changed to {} ({})", socketStateToString(socketState).toStdString(),
        socketState);

    if((socketState == QAbstractSocket::SocketState::BoundState) != _isBounded)
    {
        _isBounded = socketState == QAbstractSocket::SocketState::BoundState;
        Q_EMIT isBoundedChanged(_isBounded);
    }
}

void Worker::onSocketErrorCommon(QAbstractSocket::SocketError error, QUdpSocket* socket)
{
    if(socket)
    {
        if(error == QAbstractSocket::SocketError::ConnectionRefusedError)
        {
            LOG_DEV_WARN("Ignoring socket error ({}), because it simply mean we received an ICMP "
                         "packet Destination unreachable.",
                qPrintable(socket->errorString()));
            return;
        }
        LOG_ERR("Socket Error ({}) : {}", error, qPrintable(socket->errorString()));
        Q_EMIT socketError(error, socket->errorString());
        startWatchdog();
    }
}

QString Worker::socketStateToString(QAbstractSocket::SocketState socketState)
{
    switch(socketState)
    {
    case QAbstractSocket::UnconnectedState: return QStringLiteral("UnconnectedState");
    case QAbstractSocket::HostLookupState: return QStringLiteral("HostLookupState");
    case QAbstractSocket::ConnectingState: return QStringLiteral("ConnectingState");
    case QAbstractSocket::ConnectedState: return QStringLiteral("ConnectedState");
    case QAbstractSocket::BoundState: return QStringLiteral("BoundState");
    case QAbstractSocket::ListeningState: return QStringLiteral("ListeningState");
    case QAbstractSocket::ClosingState: return QStringLiteral("ClosingState");
    default:;
    }
    return QStringLiteral("Unknown");
}

void Worker::startBytesCounter()
{
    Q_ASSERT(_bytesCounterTimer.get() == nullptr);

    _bytesCounterTimer = std::make_unique<QTimer>();
    _bytesCounterTimer->setTimerType(Qt::TimerType::VeryCoarseTimer);
    _bytesCounterTimer->setSingleShot(false);
    _bytesCounterTimer->setInterval(1000);
    connect(_bytesCounterTimer.get(), &QTimer::timeout, this,
        [this]()
        {
            Q_EMIT rxBytesCounterChanged(_rxBytesCounter);
            Q_EMIT txBytesCounterChanged(_txBytesCounter);
            Q_EMIT rxPacketsCounterChanged(_rxPacketsCounter);
            Q_EMIT txPacketsCounterChanged(_txPacketsCounter);
            Q_EMIT rxInvalidPacketsCounterChanged(_rxInvalidPacket);

            _rxBytesCounter = 0;
            _txBytesCounter = 0;
            _rxPacketsCounter = 0;
            _txPacketsCounter = 0;
            _rxInvalidPacket = 0;
        });
    _bytesCounterTimer->start();
}

void Worker::stopBytesCounter()
{
    Q_EMIT rxBytesCounterChanged(0);
    Q_EMIT txBytesCounterChanged(0);
    Q_EMIT rxPacketsCounterChanged(0);
    Q_EMIT txPacketsCounterChanged(0);

    _bytesCounterTimer = nullptr;
}
