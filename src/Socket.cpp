// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Headers
#include <Net/Udp/Socket.hpp>
#include <Net/Udp/Worker.hpp>
#include <Net/Udp/Logger.hpp>

// Qt Headers
#include <QtCore/QThread>
#include <QtNetwork/QHostAddress>

// Stl Headers
#include <limits>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net::udp {

// clang-format off
#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
# define LOG_DEV_DEBUG(str, ...) Logger::SERVER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  do {} while (0)
#else
# define LOG_DEV_INFO(str, ...)  Logger::SERVER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  do {} while (0)
#else
# define LOG_DEV_WARN(str, ...)  Logger::SERVER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   do {} while (0)
#else
# define LOG_DEV_ERR(str, ...)   Logger::SERVER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#define LOG_DEBUG(str, ...)      Logger::SERVER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_INFO(str, ...)       Logger::SERVER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_WARN(str, ...)       Logger::SERVER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_ERR(str, ...)        Logger::SERVER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__)
// clang-format on

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

Socket::Socket(QObject* parent) : ISocket(parent) { LOG_DEV_DEBUG("Constructor"); }

Socket::~Socket()
{
    LOG_DEV_DEBUG("Destructor");

    killWorker();
}

void Socket::killWorker()
{
    if(!_worker)
        return;

    LOG_DEV_INFO("Stop Worker [{}]", static_cast<void*>(_worker));
    Q_EMIT stopWorker();

    LOG_DEV_INFO("Disconnect Worker [{}]", static_cast<void*>(_worker));
    Q_ASSERT(_worker);
    disconnect(_worker, nullptr, this, nullptr);
    disconnect(this, nullptr, _worker, nullptr);

    if(_workerThread)
    {
        // Ask to delete later in the event loop
        LOG_DEV_INFO("Kill worker thread ...");
        _workerThread->quit();
        _workerThread->wait();
        _workerThread->deleteLater();
        _workerThread = nullptr;
        // Worker will be deleted with the finished signal from QThread
        _worker = nullptr;
        LOG_DEV_INFO("... Done");
    }
    else if(_worker)
    {
        LOG_DEV_INFO("Delete worker [{}] later", static_cast<void*>(_worker));
        _worker->deleteLater();
        _worker = nullptr;
    }
}

bool Socket::setUseWorkerThread(const bool& enabled)
{
    if(ISocket::setUseWorkerThread(enabled))
    {
        if(isRunning())
        {
            LOG_DEV_INFO("Use worker thread change to {}, restart server to reflect change on worker.", enabled);
        }
        else
        {
            LOG_DEV_INFO("Use worker thread change to {}.", enabled);
        }

        if(isRunning())
            restart();
        return true;
    }
    return false;
}

QStringList Socket::multicastGroups() const
{
    return QList<QString>(_multicastListeningGroups.begin(), _multicastListeningGroups.end());
}

bool Socket::setMulticastGroups(const QStringList& value)
{
    leaveAllMulticastGroups();
    for(const auto& group: value) { joinMulticastGroup(group); }
    return true;
}

QStringList Socket::multicastListeningInterfaces() const
{
    return QList<QString>(_multicastListeningInterfaces.begin(), _multicastListeningInterfaces.end());
}

bool Socket::setMulticastListeningInterfaces(const QStringList& values)
{
    bool changedHappened = false;

    // Join the new interfaces
    for(const auto& interfaceName: values)
    {
        if(_multicastListeningInterfaces.find(interfaceName) == _multicastListeningInterfaces.end())
        {
            changedHappened = true;
            joinMulticastInterface(interfaceName);
        }
    }

    // Leave interfaces not present anymore
    std::vector<QString> interfacesToLeave;
    for(const auto& interfaceName: _multicastListeningInterfaces)
    {
        if(!values.contains(interfaceName))
        {
            changedHappened = true;
            interfacesToLeave.push_back(interfaceName);
        }
    }
    for(const auto& interfaceName: interfacesToLeave) { leaveMulticastInterface(interfaceName); }

    return changedHappened;
}

bool Socket::start()
{
    LOG_INFO("Start");
    if(isRunning())
    {
        LOG_DEV_WARN("Can't start socket that is already running. Please call 'stop' before.");
        return false;
    }

    setRunning(true);

    Q_ASSERT(_worker == nullptr);
    Q_ASSERT(_workerThread == nullptr);

    LOG_DEV_DEBUG("Create worker");

    _worker = createWorker();

    if(useWorkerThread())
    {
        _workerThread = new QThread(this);

        if(objectName().size())
            _workerThread->setObjectName(objectName() + " Worker");
        else
            _workerThread->setObjectName("UdpSocket Thread");

        _worker->moveToThread(_workerThread);
        connect(_workerThread, &QThread::finished, _worker, &QObject::deleteLater);
    }
    else
    {
        _worker->setParent(this);
    }

    _worker->setObjectName("Udp Worker");

    LOG_DEV_DEBUG("Connect to worker {}", static_cast<void*>(_worker));

    _worker->initialize(watchdogPeriod(), rxAddress(), rxPort(), txPort(), separateRxTxSockets(),
        _multicastListeningGroups, _multicastListeningInterfaces, _multicastOutgoingInterfaces, inputEnabled(),
        multicastLoopback());

    connect(this, &Socket::startWorker, _worker, &Worker::onStart);
    connect(this, &Socket::stopWorker, _worker, &Worker::onStop);
    connect(this, &Socket::restartWorker, _worker, &Worker::onRestart);

    connect(this, &Socket::joinMulticastGroupWorker, _worker, &Worker::joinMulticastGroup);
    connect(this, &Socket::leaveMulticastGroupWorker, _worker, &Worker::leaveMulticastGroup);

    connect(this, &Socket::joinMulticastInterfaceWorker, _worker, &Worker::joinMulticastInterface);
    connect(this, &Socket::leaveMulticastInterfaceWorker, _worker, &Worker::leaveMulticastInterface);

    connect(this, &Socket::rxAddressChanged, _worker, &Worker::setAddress);
    connect(this, &Socket::rxPortChanged, _worker, &Worker::setRxPort);
    connect(this, &Socket::txPortChanged, _worker, &Worker::setTxPort);
    connect(this, &Socket::separateRxTxSocketsChanged, _worker, &Worker::setSeparateRxTxSockets);
    connect(this, &Socket::multicastLoopbackChanged, _worker, &Worker::setMulticastLoopback);
    connect(this, &Socket::multicastOutgoingInterfacesChanged, _worker, &Worker::setMulticastOutgoingInterfaces);
    connect(this, &Socket::inputEnabledChanged, _worker, &Worker::setInputEnabled);
    connect(this, &Socket::watchdogPeriodChanged, _worker, &Worker::setWatchdogTimeout);

    connect(this, &Socket::sendDatagramToWorker, _worker, &Worker::onSendDatagram, Qt::QueuedConnection);
    connect(_worker, &Worker::datagramReceived, this, &Socket::onDatagramReceived, Qt::QueuedConnection);

    connect(_worker, &Worker::isBoundedChanged, this, &Socket::setBounded);
    connect(_worker, &Worker::socketError, this, &Socket::socketError);

    connect(_worker, &Worker::rxBytesCounterChanged, this, &Socket::onWorkerRxPerSecondsChanged);
    connect(_worker, &Worker::txBytesCounterChanged, this, &Socket::onWorkerTxPerSecondsChanged);
    connect(_worker, &Worker::rxPacketsCounterChanged, this, &Socket::onWorkerPacketsRxPerSecondsChanged);
    connect(_worker, &Worker::txPacketsCounterChanged, this, &Socket::onWorkerPacketsTxPerSecondsChanged);

    connect(_worker, &Worker::multicastGroupJoined, this, &Socket::multicastGroupJoined);
    connect(_worker, &Worker::multicastGroupLeaved, this, &Socket::multicastGroupLeaved);

    if(_workerThread)
    {
        LOG_DEV_INFO("Start worker thread");
        _workerThread->start();
    }

    LOG_DEV_INFO("Start worker");
    Q_EMIT startWorker();

    return true;
}

bool Socket::start(quint16 port)
{
    setRxPort(port);
    return start();
}

bool Socket::start(const QString& address, quint16 port)
{
    setRxAddress(address);
    return start(port);
}

bool Socket::restart()
{
    stop();
    return start();
}

bool Socket::stop()
{
    LOG_DEV_INFO("Stop");
    if(!isRunning())
        return false;

    setBounded(false);
    setRunning(false);

    resetRxBytesPerSeconds();
    resetTxBytesPerSeconds();
    resetRxPacketsPerSeconds();
    resetTxPacketsPerSeconds();

    _cache.clear();

    killWorker();

    return true;
}

bool Socket::joinMulticastGroup(const QString& groupAddress)
{
    // ) Check that the address isn't already registered
    if(_multicastListeningGroups.find(groupAddress) != _multicastListeningGroups.end())
        return false;

    // ) Check that this is a real multicast address
    if(!QHostAddress(groupAddress).isMulticast())
        return false;

    // ) Insert in the set and emit signal to say the multicast list changed
    _multicastListeningGroups.insert(groupAddress);
    Q_EMIT multicastGroupsChanged(multicastGroups());

    LOG_DEV_INFO("Join multicast group {} request", qPrintable(groupAddress));
    Q_EMIT joinMulticastGroupWorker(groupAddress);
    return true;
}

bool Socket::leaveMulticastGroup(const QString& groupAddress)
{
    const auto it = _multicastListeningGroups.find(groupAddress);

    // ) Is the multicast group present
    if(it == _multicastListeningGroups.end())
        return false;

    // ) Remove the multicast address from the list, then emit a signal to say the list changed
    _multicastListeningGroups.erase(it);
    Q_EMIT multicastGroupsChanged(multicastGroups());

    LOG_DEV_INFO("Leave multicast group {} request", qPrintable(groupAddress));
    Q_EMIT leaveMulticastGroupWorker(groupAddress);
    return true;
}

bool Socket::leaveAllMulticastGroups()
{
    bool allSuccess = true;
    while(!_multicastListeningGroups.empty())
    {
        // Copy is required here because leaveMulticastGroup will erase the iterator
        const auto group = *_multicastListeningGroups.begin();
        if(!leaveMulticastGroup(group))
            allSuccess = false;
    }
    return allSuccess;
}

bool Socket::isMulticastGroupPresent(const QString& groupAddress)
{
    return _multicastListeningGroups.find(groupAddress) != _multicastListeningGroups.end();
}

bool Socket::joinMulticastInterface(const QString& name)
{
    // ) Check that the address isn't already registered
    if(_multicastListeningInterfaces.find(name) != _multicastListeningInterfaces.end())
        return false;

    // ) Insert in the set and emit signal to say the multicast list changed
    _multicastListeningInterfaces.insert(name);
    Q_EMIT multicastListeningInterfacesChanged(multicastListeningInterfaces());

    LOG_DEV_INFO("Join interface for multicast {}", qPrintable(name));
    Q_EMIT joinMulticastInterfaceWorker(name);
    return true;
}

bool Socket::leaveMulticastInterface(const QString& name)
{
    const auto it = _multicastListeningInterfaces.find(name);

    // ) Is the multicast interface present
    if(it == _multicastListeningInterfaces.end())
        return false;

    // ) Remove the multicast address from the list, then emit a signal to say the list changed
    _multicastListeningInterfaces.erase(it);
    Q_EMIT multicastListeningInterfacesChanged(multicastListeningInterfaces());

    LOG_DEV_INFO("Leave interface for multicast {}", qPrintable(name));
    Q_EMIT leaveMulticastInterfaceWorker(name);
    return true;
}

bool Socket::leaveAllMulticastInterfaces()
{
    bool allSuccess = true;
    while(!_multicastListeningInterfaces.empty())
    {
        // Copy is required here because leaveMulticastInterface will erase the iterator
        const auto interface = *_multicastListeningInterfaces.begin();
        if(!leaveMulticastInterface(interface))
            allSuccess = false;
    }
    return allSuccess;
}

bool Socket::isMulticastInterfacePresent(const QString& name)
{
    return _multicastListeningInterfaces.find(name) != _multicastListeningInterfaces.end();
}

void Socket::clearRxCounter()
{
    resetRxPacketsPerSeconds();
    resetRxPacketsTotal();
    resetRxBytesPerSeconds();
    resetRxBytesTotal();
}

void Socket::clearTxCounter()
{
    resetTxPacketsPerSeconds();
    resetTxPacketsTotal();
    resetTxBytesPerSeconds();
    resetTxBytesTotal();
}

void Socket::clearRxInvalidCounter() { resetRxInvalidPacketTotal(); }

void Socket::clearCounters()
{
    clearRxCounter();
    clearTxCounter();
}

Worker* Socket::createWorker() { return new Worker; }

std::shared_ptr<Datagram> Socket::makeDatagram(const size_t length) { return _cache.make(length); }

bool Socket::sendDatagram(
    const uint8_t* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl)
{
    if(!isRunning() && !isBounded())
    {
        if(!isRunning())
            LOG_DEV_ERR("Error: Fail to send datagram because the Udp Server isn't running");
        else if(!isBounded())
            LOG_ERR("Error: Fail to send datagram because the Udp Server isn't bounded to any "
                    "interfaces.");
        return false;
    }

    if(length <= 0)
    {
        LOG_DEV_ERR("Error: Fail to send datagram because the length is <= 0");
        return false;
    }

    auto datagram = makeDatagram(length);
    memcpy(datagram->buffer(), buffer, length);
    datagram->destinationAddress = address;
    datagram->destinationPort = port;
    datagram->ttl = ttl;

    Q_EMIT sendDatagramToWorker(std::move(datagram));

    return true;
}

bool Socket::sendDatagram(
    const char* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl)
{
    return sendDatagram(reinterpret_cast<const uint8_t*>(buffer), length, address, port, ttl);
}

bool Socket::sendDatagram(
    std::shared_ptr<Datagram> datagram, const QString& address, const uint16_t port, const uint8_t ttl)
{
    if(!datagram)
    {
        LOG_DEV_ERR("Error: Fail to send null datagram");
        return false;
    }

    datagram->destinationAddress = address;
    datagram->destinationPort = port;
    datagram->ttl = ttl;
    return sendDatagram(std::move(datagram));
}

bool Socket::sendDatagram(std::shared_ptr<Datagram> datagram)
{
    if(!datagram)
    {
        LOG_DEV_ERR("Error: Fail to send null datagram");
        return false;
    }

    if(!isRunning() && !isBounded())
    {
        if(!isRunning())
            LOG_DEV_ERR("Error: Fail to send datagram because the Udp Server isn't running");
        else if(!isBounded())
            LOG_ERR("Error: Fail to send datagram because the Udp Server isn't bounded to any "
                    "interfaces.");
        return false;
    }

    if(datagram->length() <= 0)
    {
        LOG_DEV_ERR("Error: Fail to send datagram because the length is <= 0");
        return false;
    }

    Q_EMIT sendDatagramToWorker(std::move(datagram));

    return true;
}

bool Socket::sendDatagram(QJSValue datagram)
{
    static const auto addressKey = "address";
    static const auto portKey = "port";
    static const auto ttlKey = "ttl";
    static const auto dataKey = "data";

    QString address;
    quint16 port = 0;
    quint8 ttl = 0;
    SharedDatagram sharedDatagram;

    if(const auto property = datagram.property(addressKey); property.isString())
    {
        address = property.toString();
    }
    else
    {
        LOG_WARN("Can't send datagram because no 'address' specified");
        return false;
    }

    if(const auto property = datagram.property(portKey); property.isNumber())
    {
        const auto rawPort = property.toUInt();
        if(rawPort > std::numeric_limits<quint16>::max())
        {
            LOG_WARN("Can't send datagram because 'port' is out of bound ({})", rawPort);
            return false;
        }
        port = quint16(property.toUInt());
    }
    else
    {
        LOG_WARN("Can't send datagram because no 'port' specified");
        return false;
    }

    if(const auto property = datagram.property(ttlKey); property.isNumber())
    {
        const auto rawTtl = property.toUInt();
        if(rawTtl > std::numeric_limits<quint8>::max())
        {
            LOG_WARN("'ttl' will be default to os because the value ({}) is out of bound", rawTtl);
            ttl = 0;
        }
        else
        {
            ttl = quint8(property.toUInt());
        }
    }

    if(const auto property = datagram.property(dataKey); !property.isUndefined())
    {
        if(property.isString())
        {
            const auto dataAsString = property.toString();
            const auto dataAsBytes = dataAsString.toLatin1();
            sharedDatagram = makeDatagram(dataAsBytes.length());
            memcpy(sharedDatagram->buffer(), dataAsBytes.constData(), dataAsBytes.length());
        }
        else if(property.isArray())
        {
            const int length = property.property("length").toInt();
            sharedDatagram = makeDatagram(length);
            for(int i = 0; i < length; ++i) { sharedDatagram->buffer()[i] = property.property(i).toUInt(); }
        }
        else
        {
            LOG_WARN("Can't send datagram because no 'data' is unknown type");
            return false;
        }
    }
    else
    {
        LOG_WARN("Can't send datagram because no 'data' specified");
        return false;
    }
    Q_ASSERT(sharedDatagram);

    sharedDatagram->destinationAddress = address;
    sharedDatagram->destinationPort = port;
    sharedDatagram->ttl = ttl;

    return sendDatagram(sharedDatagram);
}

void Socket::onDatagramReceived(const SharedDatagram& datagram)
{
    Q_CHECK_PTR(datagram.get());
    Q_EMIT sharedDatagramReceived(datagram);

    // This function violates the object-oriented principle of modularity.
    // However, if the socket isn't used in qml we can avoid doing a deep copy of the shared datagram
    static const QMetaMethod datagramReceivedSignal = QMetaMethod::fromSignal(&ISocket::datagramReceived);
    if(isSignalConnected(datagramReceivedSignal))
    {
        Q_ASSERT(datagram->length() < std::size_t(std::numeric_limits<int>::max()));
        const QJSValue jsData(
            QString::fromLatin1(reinterpret_cast<const char*>(datagram->buffer()), int(datagram->length())));
        const QJSValue jsDestinationAddress(datagram->destinationAddress);
        const QJSValue jsDestinationPort(datagram->destinationPort);
        const QJSValue jsSenderAddress(datagram->senderAddress);
        const QJSValue jsSenderPort(datagram->senderPort);
        const QJSValue jsTtl(datagram->ttl);

        QJSEngine* engine = qjsEngine(this);
        QJSValue jsDatagram = engine->newObject();

        jsDatagram.setProperty("data", jsData);
        jsDatagram.setProperty("destinationAddress", jsDestinationAddress);
        jsDatagram.setProperty("destinationPort", jsDestinationPort);
        jsDatagram.setProperty("senderAddress", jsSenderAddress);
        jsDatagram.setProperty("senderPort", jsSenderPort);
        jsDatagram.setProperty("ttl", jsTtl);

        Q_EMIT datagramReceived(jsDatagram);
    }
}

void Socket::onWorkerRxPerSecondsChanged(const quint64 rxBytes)
{
    setRxBytesPerSeconds(rxBytes);
    setRxBytesTotal(rxBytesTotal() + rxBytes);
}

void Socket::onWorkerTxPerSecondsChanged(const quint64 txBytes)
{
    setTxBytesPerSeconds(txBytes);
    setTxBytesTotal(txBytesTotal() + txBytes);
}

void Socket::onWorkerPacketsRxPerSecondsChanged(const quint64 rxPackets)
{
    setRxPacketsPerSeconds(rxPackets);
    setRxPacketsTotal(rxPacketsTotal() + rxPackets);
}

void Socket::onWorkerPacketsTxPerSecondsChanged(const quint64 txPackets)
{
    setTxPacketsPerSeconds(txPackets);
    setTxPacketsTotal(txPacketsTotal() + txPackets);
}

void Socket::onWorkerRxInvalidPacketsCounterChanged(const quint64 rxPackets)
{
    setRxInvalidPacketTotal(rxInvalidPacketTotal() + rxPackets);
}

}
