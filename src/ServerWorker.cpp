// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header
#include <QLoggingCategory>
#include <QTimer>
#include <QUdpSocket>
#include <QCoreApplication>
#include <QNetworkDatagram>

// Dependencies Header

// Application Header
#include <NetUdp/ServerWorker.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(NETUDP_SERVERWORKER_LOGCAT, "net.udp.server.worker")

NETUDP_USING_NAMESPACE;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

ServerWorker::ServerWorker(QObject* parent): QObject(parent)
{
}

ServerWorker::~ServerWorker()
{
}

void ServerWorker::onRestart()
{
    onStop();
    onStart();
}

void ServerWorker::onStart()
{
    if (_socket)
    {
        qCWarning(NETUDP_SERVERWORKER_LOGCAT, "Error : Can't start udp server worker because socket is already valid");
        return;
    }

    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Start Udp Server Worker");

    _isBounded = false;

    // ) Create the socket
    _socket = std::make_unique<QUdpSocket>(this);

    // ) Connect to socket signals
    connect(_socket.get(), &QUdpSocket::readyRead, this, &ServerWorker::readPendingDatagrams);
    connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
            &ServerWorker::onSocketError);
    connect(_socket.get(), &QUdpSocket::stateChanged, this, &ServerWorker::onSocketStateChanged);

    // ) Create a watchdog timer
    _watchdog = std::make_unique<QTimer>();
    _watchdog->setSingleShot(true);

    // ) Connect timeout
    connect(_watchdog.get(), &QTimer::timeout, this, &ServerWorker::onWatchdogTimeout);

    // ) Bind to socket
    // When only in output mode, calling _socket->bind() doesn't allow to set multicast ttl.
    // But calling _socket->bind(QHostAddress()) allow to set the ttl.
    // From what i understand, _socket->bind() will call _socket->bind(QHostAddress::Any) internally.
    // _socket->bind(QHostAddress()) bind to a non valid host address and random port will be choose.
    // Qt multicast issues are non resolved ? https://forum.qt.io/topic/78090/multicast-issue-possible-bug/17
    const bool bindSuccess = _inputEnabled ? _socket->bind(QHostAddress(_address), _port, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint) : _socket->bind(QHostAddress(_address), 0, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

    if (bindSuccess)
    {
        if(_address.isEmpty())
            qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Success bind to port %d", _port);
        else
            qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Success bind to %s: %d", qPrintable(_address), _port);
        setMulticastInterfaceNameToSocket();
        setMulticastLoopbackToSocket();
        if(_inputEnabled)
        {
            for (auto it = _multicastGroups.begin(); it != _multicastGroups.end(); ++it)
            {
                it.value() = _multicastInterface.isValid() ? _socket->joinMulticastGroup(QHostAddress(it.key()), _multicastInterface) : _socket->joinMulticastGroup(QHostAddress(it.key()));
                if (it.value())
                    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Success join multicast group %s", qPrintable(it.key()));
                else
                    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Error : Fail to join multicast group %s", qPrintable(it.key()));
            }
        }

        startBytesCounter();
    }
    else
    {
        qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Error : Fail to bind to %s : %d", qPrintable(_address.isEmpty() ? "Any": _address), _port);
        _socket = nullptr;
        startWatchdog();
    }
}

void ServerWorker::onStop()
{
    if (!_socket)
    {
        qDebug("Error : Can't stop udp server worker because socket isn't valid");
        return;
    }

    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Stop Udp Server Worker");

    stopBytesCounter();

    _watchdog = nullptr;
    _socket = nullptr;
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
    if (address != _address)
    {
        _address = address;
        onRestart();
    }
}

void ServerWorker::setPort(const quint16 port)
{
    if (port != _port)
    {
        _port = port;
        onRestart();
    }
}

void ServerWorker::joinMulticastGroup(const QString& address)
{
    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Join Multicast group %s", qPrintable(address));

    if (!_inputEnabled)
        return;

    const auto hostAddress = QHostAddress(address);
    if (!address.isEmpty() && hostAddress.isMulticast())
    {
        if (!_multicastGroups.contains(address))
        {
            const bool successJoin = _socket && (_multicastInterface.isValid() ? _socket->joinMulticastGroup(hostAddress, _multicastInterface) : _socket->joinMulticastGroup(hostAddress));
            _multicastGroups.insert(address, successJoin);
        }
    }
}

void ServerWorker::leaveMulticastGroup(const QString& address)
{
    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Leave Multicast group %s", qPrintable(address));

    if (_multicastGroups.contains(address))
    {
        const bool successLeave = _socket && (_multicastInterface.isValid() ? _socket->leaveMulticastGroup(QHostAddress(address), _multicastInterface) : _socket->leaveMulticastGroup(QHostAddress(address)));
        _multicastGroups.remove(address);
    }
}

void ServerWorker::setMulticastInterfaceName(const QString& name)
{
    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Set Multicast Interface Name : %s", qPrintable(name));

    if (name != _multicastInterface.name())
    {
        const auto iface = QNetworkInterface::interfaceFromName(name);
        _multicastInterface = iface;
        onRestart();
    }
}

void ServerWorker::setMulticastLoopback(const bool loopback)
{
    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Set Multicast Loopback : %s", qPrintable(loopback ? "true" : "false"));

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

void ServerWorker::setMulticastInterfaceNameToSocket() const
{
    if (_socket && _multicastInterface.isValid())
    {
        qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Set outgoing interface %s for multicast packets", qPrintable(_multicastInterface.name()));
        _socket->setMulticastInterface(_multicastInterface);
        const auto i = _socket->multicastInterface();
        if (!i.isValid())
            qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Error : Can't use %s for output multicast packet", qPrintable(_multicastInterface.name()));
    }
}

void ServerWorker::setMulticastLoopbackToSocket() const
{
    if (_socket && _inputEnabled)
    {
        qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Set MulticastLoopbackOption to %d", int(_multicastLoopback));
        _socket->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _multicastLoopback);
    }
}

void ServerWorker::startWatchdog() const
{
    if (_watchdog)
        _watchdog->start(_watchdogTimeout);
}

void ServerWorker::stopWatchdog() const
{
    if (_watchdog && _watchdog->isActive())
        _watchdog->stop();
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

void ServerWorker::onSendDatagram(const SharedDatagram datagram)
{
    if (!datagram)
    {
        qCWarning(NETUDP_SERVERWORKER_LOGCAT, "Error : Can't send null datagram");
        return;
    }

    if (!_socket)
    {
        qCWarning(NETUDP_SERVERWORKER_LOGCAT, "Error : Can't send a datagram when the socket is null");
        return;
    }

    if(datagram->destinationAddress.isNull())
    {
        qCWarning(NETUDP_SERVERWORKER_LOGCAT, "Error : Can't send datagram to null address");
        return;
    }

    if (!datagram->buffer)
    {
        qCWarning(NETUDP_SERVERWORKER_LOGCAT, "Error : Can't send datagram with empty buffer");
        return;
    }

    if (!datagram->length)
    {
        qCWarning(NETUDP_SERVERWORKER_LOGCAT, "Error : Can't send datagram with data length to 0");
        return;
    }

    qint64 bytesWritten = 0;

    if (datagram->destinationAddress.isMulticast())
    {
        setMulticastTtl(datagram->ttl);
        bytesWritten = _socket->writeDatagram(reinterpret_cast<const char*>(datagram->buffer.get()), datagram->length,
                                              datagram->destinationAddress, datagram->destinationPort);
    }
    else
    {
        QNetworkDatagram d(QByteArray(reinterpret_cast<const char*>(datagram->buffer.get()), int(datagram->length)),
                           datagram->destinationAddress, datagram->destinationPort);
        if (datagram->ttl)
            d.setHopLimit(datagram->ttl);
        bytesWritten = _socket->writeDatagram(d);
    }

    if (bytesWritten <= 0)
    {
        qCWarning(NETUDP_SERVERWORKER_LOGCAT, "Error : Fail to send datagram, 0 bytes written");
        return;
    }

    if (bytesWritten != datagram->length)
    {
        qCWarning(NETUDP_SERVERWORKER_LOGCAT, "Error : Fail to send datagram, %lld/%lld bytes written", bytesWritten, int64_t(datagram->length));
        return;
    }

    _txBytesCounter += bytesWritten;
    ++_txPacketsCounter;
}

bool ServerWorker::isPacketValid(const uint8_t* buffer, const size_t length)
{
    return true;
}

void ServerWorker::readPendingDatagrams()
{
    if (!_socket)
        return;

    while (_socket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = _socket->receiveDatagram();

        if (!isPacketValid(reinterpret_cast<const uint8_t*>(datagram.data().constData()), datagram.data().size()))
        {
            qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Error : Receive not valid datagram");
            return;
        }

        SharedDatagram sharedDatagram = std::make_shared<Datagram>();
        sharedDatagram->buffer = std::make_unique<uint8_t[]>(datagram.data().size());
        memcpy(sharedDatagram->buffer.get(), reinterpret_cast<const uint8_t*>(datagram.data().constData()), datagram.data().size());
        sharedDatagram->length = datagram.data().size();
        sharedDatagram->destinationAddress = datagram.destinationAddress();
        sharedDatagram->destinationPort = datagram.destinationPort();
        sharedDatagram->senderAddress = datagram.senderAddress();
        sharedDatagram->senderPort = datagram.senderPort();
        sharedDatagram->ttl = datagram.hopLimit();

        _rxBytesCounter += datagram.data().size();
        ++_rxPacketsCounter;

        Q_EMIT receivedDatagram(sharedDatagram);
    }
}

void ServerWorker::onSocketError(QAbstractSocket::SocketError error)
{
    if (_socket)
    {
        qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Error : Socket Error (%d) : %s",
               error, qPrintable(_socket->errorString()));

        Q_EMIT socketError(error, _socket->errorString());
    }
}

void ServerWorker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    qCDebug(NETUDP_SERVERWORKER_LOGCAT, "Socket State Changed to %s (%d)", qPrintable(socketStateToString(socketState)), socketState);

    if ((socketState == QAbstractSocket::SocketState::BoundState) != _isBounded)
    {
        _isBounded = socketState == QAbstractSocket::SocketState::BoundState;
        Q_EMIT isBoundedChanged(_isBounded);
    }
}

void ServerWorker::onWatchdogTimeout()
{
    if (!_isBounded)
        onRestart();
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

    _rxBytesCounter = 0;
    _txBytesCounter = 0;
    _rxPacketsCounter = 0;
    _txPacketsCounter = 0;
}
