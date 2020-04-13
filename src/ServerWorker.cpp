// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/ServerWorker.hpp>
#include <Net/Udp/Logger.hpp>

// Qt Header
#include <QTimer>

#include <QUdpSocket>
#include <QNetworkDatagram>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Udp;

#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
# define LOG_DEV_DEBUG(str, ...) Logger::WORKER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  do {} while (0)
#else
# define LOG_DEV_INFO(str, ...)  Logger::WORKER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  do {} while (0)
#else
# define LOG_DEV_WARN(str, ...)  Logger::WORKER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   do {} while (0)
#else
# define LOG_DEV_ERR(str, ...)   Logger::WORKER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#define LOG_DEBUG(str, ...)      Logger::WORKER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_INFO(str, ...)       Logger::WORKER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_WARN(str, ...)       Logger::WORKER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_ERR(str, ...)        Logger::WORKER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__);

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

ServerWorker::ServerWorker(QObject* parent): QObject(parent)
{
    LOG_DEV_DEBUG("Constructor");
}

ServerWorker::~ServerWorker()
{
    LOG_DEV_DEBUG("Destructor");
}

bool ServerWorker::isBounded() const
{
    return _isBounded;
}

quint64 ServerWorker::watchdogTimeout() const
{
    return _watchdogTimeout;
}

QString ServerWorker::rxAddress() const
{
    return _rxAddress;
}

quint16 ServerWorker::rxPort() const
{
    return _rxPort;
}

quint16 ServerWorker::txPort() const
{
    return _txPort;
}

QMap<QString, bool> ServerWorker::multicastGroups() const
{
    return _multicastGroups;
}

QNetworkInterface ServerWorker::multicastInterface() const
{
    return _multicastInterface;
}

bool ServerWorker::multicastLoopback() const
{
    return _multicastLoopback;
}

quint8 ServerWorker::multicastTtl() const
{
    return _multicastTtl;
}

bool ServerWorker::inputEnabled() const
{
    return _inputEnabled;
}

bool ServerWorker::separateRxTxSocketsChanged() const
{
    return _separateRxTxSockets;
}

QUdpSocket* ServerWorker::rxSocket() const
{
    if (_separateRxTxSockets)
        return _rxSocket.get();
    return _socket.get();
}

size_t ServerWorker::cacheSize() const
{
    return _cache.size();
}

bool ServerWorker::resizeCache(size_t length)
{
    return _cache.resize(length);
}

void ServerWorker::clearCache()
{
    _cache.clear();
}

void ServerWorker::releaseCache()
{
    _cache.release();
}

std::shared_ptr<Datagram> ServerWorker::makeDatagram(const size_t length)
{
    return _cache.make(length);
}

void ServerWorker::onRestart()
{
    onStop();
    onStart();
}

void ServerWorker::onStart()
{
    const bool useTwoSockets = (_separateRxTxSockets || _txPort) && _inputEnabled;
    if (_socket)
    {
        LOG_DEV_ERR("Can't start udp server worker because socket is already valid");
        return;
    }

    LOG_INFO("Start Udp Server Worker");

    _isBounded = false;
    _watchdog = nullptr;

    // ) Create the socket
    _socket = std::make_unique<QUdpSocket>(this);
    if (useTwoSockets)
    {
        LOG_INFO("Create separate rx socket");
        _rxSocket = std::make_unique<QUdpSocket>(this);
    }

    // ) Connect to socket signals
    connect(rxSocket(), &QUdpSocket::readyRead, this, &ServerWorker::readPendingDatagrams);
    connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
        &ServerWorker::onSocketError);
    // _socket is always bounded because binded to QHostAddress(). Only rxSocket can go wrong
    connect(rxSocket(), &QUdpSocket::stateChanged, this, &ServerWorker::onSocketStateChanged);

    if (useTwoSockets)
    {
        connect(_rxSocket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
            &ServerWorker::onRxSocketError);
        //connect(_rxSocket.get(), &QUdpSocket::stateChanged, this, &ServerWorker::onSocketStateChanged);
    }

    // ) Bind to socket
    // When only in output mode, calling _socket->bind() doesn't allow to set multicast ttl.
    // But calling _socket->bind(QHostAddress()) allow to set the ttl.
    // From what i understand, _socket->bind() will call _socket->bind(QHostAddress::Any) internally.
    // _socket->bind(QHostAddress()) bind to a non valid host address and random port will be choose.
    // Qt multicast issues are non resolved ? https://forum.qt.io/topic/78090/multicast-issue-possible-bug/17
    bool bindSuccess = false;
    if (useTwoSockets)
        bindSuccess = _rxSocket->bind(QHostAddress(_rxAddress), _rxPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint) && _socket->bind(QHostAddress(), _txPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    else if (_inputEnabled)
        bindSuccess = _socket->bind(QHostAddress(_rxAddress), _rxPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    else
        bindSuccess = _socket->bind(QHostAddress(), _txPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

    if (bindSuccess)
    {
        if(_rxAddress.isEmpty())
        {
            LOG_INFO("Success bind to port {}", _rxPort);
        }
        else
        {
            LOG_INFO("Success bind to {}: {}", qPrintable(_rxAddress), _rxPort);
        }
        setMulticastInterfaceNameToSocket();
        setMulticastLoopbackToSocket();
        if(_inputEnabled)
        {
            for (auto it = _multicastGroups.begin(); it != _multicastGroups.end(); ++it)
            {
                it.value() = _multicastInterface.isValid() ? rxSocket()->joinMulticastGroup(QHostAddress(it.key()), _multicastInterface) : rxSocket()->joinMulticastGroup(QHostAddress(it.key()));
                if (it.value())
                {
                    LOG_INFO("Success join multicast group {}", qPrintable(it.key()));
                }
                else
                {
                    LOG_ERR("Fail to join multicast group {}", qPrintable(it.key()));
                }
            }
        }

        startBytesCounter();
    }
    else
    {
        LOG_ERR("Fail to bind to {} : {}", qPrintable(_rxAddress.isEmpty() ? "Any": _rxAddress), _rxPort);
        _socket = nullptr;
        _rxSocket = nullptr;
        startWatchdog();
    }
}

void ServerWorker::onStop()
{
    if (!_socket)
    {
        LOG_DEV_ERR("Can't stop udp server worker because socket isn't valid");
        return;
    }

    LOG_INFO("Stop Udp Server Worker");

    stopBytesCounter();
    stopWatchdog();

    _socket = nullptr;
    _rxSocket = nullptr;
    _multicastTtl = 0;

    for(auto& it: _multicastGroups)
        it = false;
}

void ServerWorker::setWatchdogTimeout(const quint64 ms)
{
    if (_watchdogTimeout != ms)
        _watchdogTimeout = ms;
}

void ServerWorker::setAddress(const QString& address)
{
    if (address != _rxAddress)
    {
        _rxAddress = address;
        onRestart();
    }
}

void ServerWorker::setRxPort(const quint16 port)
{
    if (port != _rxPort)
    {
        _rxPort = port;
        if(_inputEnabled)
            onRestart();
    }
}

void ServerWorker::setTxPort(const quint16 port)
{
    if (port != _txPort)
    {
        _txPort = port;
        onRestart();
    }
}

void ServerWorker::joinMulticastGroup(const QString& address)
{
    LOG_INFO("Join Multicast group {}", address.toStdString());

    if (!_inputEnabled)
        return;

    const auto hostAddress = QHostAddress(address);
    if (!address.isEmpty() && hostAddress.isMulticast())
    {
        if (!_multicastGroups.contains(address))
        {
            const bool successJoin = rxSocket() && (_multicastInterface.isValid() ? rxSocket()->joinMulticastGroup(hostAddress, _multicastInterface) : rxSocket()->joinMulticastGroup(hostAddress));
            _multicastGroups.insert(address, successJoin);
        }
    }
}

void ServerWorker::leaveMulticastGroup(const QString& address)
{
    LOG_INFO("Leave Multicast group {}", address.toStdString());

    if (_multicastGroups.contains(address))
    {
        const bool successLeave = rxSocket() && (_multicastInterface.isValid() ? rxSocket()->leaveMulticastGroup(QHostAddress(address), _multicastInterface) : rxSocket()->leaveMulticastGroup(QHostAddress(address)));
        _multicastGroups.remove(address);
    }
}

void ServerWorker::setMulticastInterfaceName(const QString& name)
{
    LOG_INFO("Set Multicast Interface Name : {}", name.toStdString());

    if (name != _multicastInterface.name())
    {
        const auto iface = QNetworkInterface::interfaceFromName(name);
        _multicastInterface = iface;
        onRestart();
    }
}

void ServerWorker::setMulticastLoopback(const bool loopback)
{
    LOG_INFO("Set Multicast Loopback : {}", loopback);

    if (_multicastLoopback != loopback)
    {
        _multicastLoopback = loopback;
        setMulticastLoopbackToSocket();
    }
}

void ServerWorker::setInputEnabled(const bool enabled)
{
    if(enabled != _inputEnabled)
    {
        _inputEnabled = enabled;
        onRestart();
    }
}

void ServerWorker::setSeparateRxTxSockets(const bool separateRxTxSocketsChanged)
{
    const bool shouldUseSeparate = separateRxTxSocketsChanged || _txPort;
    if (shouldUseSeparate != _separateRxTxSockets)
    {
        _separateRxTxSockets = shouldUseSeparate;
        if(_inputEnabled)
            onRestart();
    }
}

void ServerWorker::setMulticastInterfaceNameToSocket() const
{
    if (_socket && _multicastInterface.isValid())
    {
        LOG_INFO("Set outgoing interface {} for multicast packets", _multicastInterface.name().toStdString());
        _socket->setMulticastInterface(_multicastInterface);
        const auto i = _socket->multicastInterface();
        if (!i.isValid())
            LOG_ERR("Can't use {} as output multicast interface", _multicastInterface.name().toStdString());
    }
}

void ServerWorker::setMulticastLoopbackToSocket() const
{
    if (rxSocket() && _inputEnabled)
    {
        LOG_INFO("Set MulticastLoopbackOption to {}", int(_multicastLoopback));
        rxSocket()->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _multicastLoopback);
    }
}

void ServerWorker::startWatchdog()
{
    if(!_watchdog)
    {
        onStop();

        // ) Create a watchdog timer
        _watchdog = std::make_unique<QTimer>();
        _watchdog->setTimerType(Qt::TimerType::VeryCoarseTimer);

        // ) Connect timeout
        connect(_watchdog.get(), &QTimer::timeout, this,
            [this]()
            {
                onRestart();
            }, Qt::ConnectionType::QueuedConnection
        );

        LOG_INFO("Start watchdog to restart socket in {} millis", watchdogTimeout())
        // Start the watchdog
        _watchdog->start(watchdogTimeout());
    }
    else
    {
        LOG_DEV_WARN("Watchdog already running, fail to start it");
    }
}

void ServerWorker::stopWatchdog()
{
    _watchdog = nullptr;
}

void ServerWorker::setMulticastTtl(const quint8 ttl)
{
    if (!_socket)
        return;

    if (!ttl)
        return;

    if (ttl != _multicastTtl)
    {
        _socket->setSocketOption(QAbstractSocket::MulticastTtlOption, int(ttl));
        _multicastTtl = ttl;
    }
}

void ServerWorker::onSendDatagram(const SharedDatagram& datagram)
{
    if(!isBounded())
    {
        LOG_DEV_ERR("Can't send datagram if socket isn't bounded");
        return;
    }
    if (!datagram)
    {
        LOG_DEV_ERR("Can't send null datagram");
        return;
    }

    if (!_socket)
    {
        LOG_ERR("Can't send a datagram when the socket is null");
        return;
    }

    if(datagram->destinationAddress.isNull())
    {
        LOG_ERR("Can't send datagram to null address");
        return;
    }

    if (!datagram->buffer())
    {
        LOG_ERR("Can't send datagram with empty buffer");
        return;
    }

    if (!datagram->length())
    {
        LOG_DEV_ERR("Can't send datagram with data length to 0");
        return;
    }

    qint64 bytesWritten = 0;

    if (datagram->destinationAddress.isMulticast())
    {
        setMulticastTtl(datagram->ttl);
        bytesWritten = _socket->writeDatagram(reinterpret_cast<const char*>(datagram->buffer()), datagram->length(),
                                              datagram->destinationAddress, datagram->destinationPort);
    }
    else
    {
        // Copy will happen :(
        // Don't have other choice in order to set ttl
        QNetworkDatagram d(QByteArray(reinterpret_cast<const char*>(datagram->buffer()), int(datagram->length())),
                           datagram->destinationAddress, datagram->destinationPort);
        if (datagram->ttl)
            d.setHopLimit(datagram->ttl);
        bytesWritten = _socket->writeDatagram(d);
    }

    if (bytesWritten <= 0)
    {
        LOG_ERR("Fail to send datagram, 0 bytes written. Restart Socket.");
        startWatchdog();
        return;
    }

    if (bytesWritten != datagram->length())
    {
        LOG_ERR("Fail to send datagram, {}/{} bytes written. Restart Socket.", static_cast<long long>(bytesWritten), static_cast<long long>(datagram->length()));
        startWatchdog();
        return;
    }

    _txBytesCounter += bytesWritten;
    ++_txPacketsCounter;
}

bool ServerWorker::isPacketValid(const uint8_t* buffer, const size_t length) const
{
    return buffer && length;
}

void ServerWorker::readPendingDatagrams()
{
    if (!rxSocket())
        return;

    while (rxSocket() && rxSocket()->isValid() && rxSocket()->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = rxSocket()->receiveDatagram();

        if(!datagram.isValid())
        {
            LOG_ERR("Receive datagram that is marked not valid. Restart Socket."
            "This may be a sign that your OS doesn't support IGMP. On unix system you can check with netstat -g");
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

        if (!isPacketValid(reinterpret_cast<const uint8_t*>(datagram.data().constData()), datagram.data().size()))
        {
            LOG_WARN("Receive not valid application datagram. Simply discard the packet");
            ++_rxInvalidPacket;
            continue;
        }

        if(datagram.data().size() > 65535)
        {
            LOG_ERR("Receive a datagram with size of {}, that is too big for a datagram. Restart Socket.", datagram.data().size());
            ++_rxInvalidPacket;
            startWatchdog();
            return;
        }

        SharedDatagram sharedDatagram = makeDatagram(datagram.data().size());
        memcpy(sharedDatagram.get()->buffer(), reinterpret_cast<const uint8_t*>(datagram.data().constData()), datagram.data().size());
        sharedDatagram->destinationAddress = datagram.destinationAddress();
        sharedDatagram->destinationPort = datagram.destinationPort();
        sharedDatagram->senderAddress = datagram.senderAddress();
        sharedDatagram->senderPort = datagram.senderPort();
        sharedDatagram->ttl = datagram.hopLimit();

        _rxBytesCounter += datagram.data().size();
        ++_rxPacketsCounter;

        onDatagramReceived(sharedDatagram);
    }
}

void ServerWorker::onDatagramReceived(const SharedDatagram& datagram)
{
    Q_EMIT datagramReceived(datagram);
}

void ServerWorker::onSocketError(QAbstractSocket::SocketError error)
{
    if (_socket)
    {
        LOG_ERR("Socket Error ({}) : {}",
               error, qPrintable(_socket->errorString()));

        Q_EMIT socketError(error, _socket->errorString());
    }
}

void ServerWorker::onRxSocketError(QAbstractSocket::SocketError error)
{
    if (_socket)
    {
        LOG_ERR("Rx Socket Error ({}) : {}",
            error, qPrintable(_rxSocket->errorString()));

        Q_EMIT socketError(error, _rxSocket->errorString());

        startWatchdog();
    }
}

void ServerWorker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    LOG_INFO("Socket State Changed to {} ({})", socketStateToString(socketState).toStdString(), socketState);

    if ((socketState == QAbstractSocket::SocketState::BoundState) != _isBounded)
    {
        _isBounded = socketState == QAbstractSocket::SocketState::BoundState;
        Q_EMIT isBoundedChanged(_isBounded);
    }
}

QString ServerWorker::socketStateToString(QAbstractSocket::SocketState socketState)
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
    default: ;
    }
    return QStringLiteral("Unknown");
}

void ServerWorker::startBytesCounter()
{
    Q_ASSERT(_bytesCounterTimer.get() == nullptr);

    _bytesCounterTimer = std::make_unique<QTimer>();
    _bytesCounterTimer->setTimerType(Qt::TimerType::VeryCoarseTimer);
    _bytesCounterTimer->setSingleShot(false);
    _bytesCounterTimer->setInterval(1000);
    connect(_bytesCounterTimer.get(), &QTimer::timeout, this, &ServerWorker::updateDataCounter);
    _bytesCounterTimer->start();
}

void ServerWorker::stopBytesCounter()
{
    Q_EMIT rxBytesCounterChanged(0);
    Q_EMIT txBytesCounterChanged(0);
    Q_EMIT rxPacketsCounterChanged(0);
    Q_EMIT txPacketsCounterChanged(0);

    _bytesCounterTimer = nullptr;
}

void ServerWorker::updateDataCounter()
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
}
