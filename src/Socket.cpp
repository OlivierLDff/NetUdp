// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Socket.hpp>
#include <Net/Udp/Worker.hpp>
#include <Net/Udp/Logger.hpp>

// Qt Header
#include <QThread>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace net::udp;

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
    // ) We can't destroy a thread that is running
    if(_workerThread)
    {
        LOG_DEV_INFO("Kill worker thread in destructor ...");
        _workerThread->exit();
        _workerThread->wait();
        LOG_DEV_INFO("Done");
    }
}

bool Socket::setUseWorkerThread(const bool& enabled)
{
    if(ISocket::setUseWorkerThread(enabled))
    {
        if(isRunning())
            LOG_DEV_INFO(
                "Use worker thread change to {}, restart server to reflect change on worker.",
                enabled);
        else
            LOG_DEV_INFO("Use worker thread change to {}.", enabled);

        if(isRunning())
            restart();
        return true;
    }
    return false;
}

bool Socket::setMulticastInterfaceName(const QString& name)
{
    return (name.isEmpty() || QNetworkInterface::interfaceFromName(name).isValid()) &&
           ISocket::setMulticastInterfaceName(name);
}

bool Socket::setMulticastGroups(const QStringList& value)
{
    leaveAllMulticastGroups();
    for(const auto& it: value) { joinMulticastGroup(it); }
    return true;
}

QStringList Socket::multicastGroups() const
{
    QStringList res;
    for(const auto& it: _multicastGroups) res.append(it);
    return res;
}

bool Socket::start()
{
    LOG_INFO("Start");
    if(isRunning())
        return false;

    setRunning(true);

    Q_ASSERT(_worker.get() == nullptr);
    Q_ASSERT(_workerThread.get() == nullptr);

    LOG_DEV_DEBUG("Create worker");
    _worker = createWorker();
    if(useWorkerThread())
    {
        LOG_DEV_DEBUG("Move worker to its own thread");
        _workerThread = std::make_unique<QThread>();

        connect(_workerThread.get(), &QThread::finished, this, [this]() { _worker = nullptr; });

        if(objectName().size())
            _workerThread->setObjectName(objectName() + " Worker");
        else
            _workerThread->setObjectName("Server Worker");
        _worker->moveToThread(_workerThread.get());
    }

    LOG_DEV_DEBUG("Connect to worker {}", static_cast<void*>(_worker.get()));

    _worker->_watchdogTimeout = watchdogPeriod();
    _worker->_rxAddress = rxAddress();
    _worker->_rxPort = rxPort();
    _worker->_txPort = txPort();

    for(const auto& it: _multicastGroups) _worker->_multicastGroups.insert(it, false);

    _worker->_separateRxTxSockets = separateRxTxSockets() || txPort();
    _worker->_multicastInterface = QNetworkInterface::interfaceFromName(multicastInterfaceName());
    _worker->_multicastLoopback = multicastLoopback();
    _worker->_inputEnabled = inputEnabled();

    connect(this, &Socket::startWorker, _worker.get(), &Worker::onStart);
    connect(this, &Socket::stopWorker, _worker.get(), &Worker::onStop);
    connect(this, &Socket::restartWorker, _worker.get(), &Worker::onRestart);

    connect(this, &Socket::joinMulticastGroupWorker, _worker.get(), &Worker::joinMulticastGroup);
    connect(this, &Socket::leaveMulticastGroupWorker, _worker.get(), &Worker::leaveMulticastGroup);

    connect(this, &Socket::rxAddressChanged, _worker.get(), &Worker::setAddress);
    connect(this, &Socket::rxPortChanged, _worker.get(), &Worker::setRxPort);
    connect(this, &Socket::txPortChanged, _worker.get(), &Worker::setTxPort);
    connect(
        this, &Socket::separateRxTxSocketsChanged, _worker.get(), &Worker::setSeparateRxTxSockets);
    connect(this, &Socket::multicastLoopbackChanged, _worker.get(), &Worker::setMulticastLoopback);
    connect(this, &Socket::multicastInterfaceNameChanged, _worker.get(),
        &Worker::setMulticastInterfaceName);
    connect(this, &Socket::inputEnabledChanged, _worker.get(), &Worker::setInputEnabled);
    connect(this, &Socket::watchdogPeriodChanged, _worker.get(), &Worker::setWatchdogTimeout);

    connect(this, &Socket::sendDatagramToWorker, _worker.get(), &Worker::onSendDatagram);
    connect(_worker.get(), &Worker::datagramReceived, this, &Socket::onDatagramReceived);

    connect(_worker.get(), &Worker::isBoundedChanged, this, &Socket::setBounded);
    connect(_worker.get(), &Worker::socketError, this, &Socket::socketError);

    connect(
        _worker.get(), &Worker::rxBytesCounterChanged, this, &Socket::onWorkerRxPerSecondsChanged);
    connect(
        _worker.get(), &Worker::txBytesCounterChanged, this, &Socket::onWorkerTxPerSecondsChanged);
    connect(_worker.get(), &Worker::rxPacketsCounterChanged, this,
        &Socket::onWorkerPacketsRxPerSecondsChanged);
    connect(_worker.get(), &Worker::txPacketsCounterChanged, this,
        &Socket::onWorkerPacketsTxPerSecondsChanged);

    if(_workerThread)
    {
        LOG_DEV_DEBUG("Start worker thread");
        _workerThread->start();
    }

    LOG_DEV_DEBUG("Start worker");
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
    LOG_DEV_DEBUG("Stop");
    if(!isRunning())
        return false;

    setBounded(false);
    setRunning(false);

    resetRxBytesPerSeconds();
    resetTxBytesPerSeconds();
    resetRxPacketsPerSeconds();
    resetTxPacketsPerSeconds();

    _cache.clear();

    LOG_DEV_DEBUG("Stop Worker");
    Q_EMIT stopWorker();

    LOG_DEV_DEBUG("Disconnect worker");

    disconnect(_worker.get(), nullptr, this, nullptr);
    disconnect(this, nullptr, _worker.get(), nullptr);

    if(_workerThread)
    {
        LOG_DEV_DEBUG("Kill worker thread ...");
        _workerThread->exit();
        _workerThread->wait();
        _workerThread = nullptr;
        LOG_DEV_DEBUG("... Done");
    }
    else
    {
        LOG_DEV_DEBUG("Delete worker later");
        // Reparent to self in case there are no event loop
        _worker->setParent(this);
        // Ask to delete later in the event loop
        _worker.release()->deleteLater();
    }

    return true;
}

bool Socket::joinMulticastGroup(const QString& groupAddress)
{
    // ) Check that the address isn't already registered
    if(_multicastGroups.find(groupAddress) != _multicastGroups.end())
        return false;

    // ) Check that this is a real multicast address
    if(!QHostAddress(groupAddress).isMulticast())
        return false;

    // ) Insert in the set and emit signal to say the multicast list changed
    _multicastGroups.insert(groupAddress);
    Q_EMIT multicastGroupsChanged(multicastGroups());

    LOG_DEV_INFO("Join multicast group %s request", qPrintable(groupAddress));
    Q_EMIT joinMulticastGroupWorker(groupAddress);
    return true;
}

bool Socket::leaveMulticastGroup(const QString& groupAddress)
{
    const auto it = _multicastGroups.find(groupAddress);

    // ) Is the multicast group present
    if(it == _multicastGroups.end())
        return false;

    // ) Remove the multicast address from the list, then emit a signal to say the list changed
    _multicastGroups.erase(it);
    Q_EMIT multicastGroupsChanged(multicastGroups());

    LOG_DEV_INFO("Leave multicast group %s request", qPrintable(groupAddress));
    Q_EMIT leaveMulticastGroupWorker(groupAddress);
    return true;
}

bool Socket::leaveAllMulticastGroups()
{
    bool allSuccess = true;
    while(!_multicastGroups.empty())
    {
        // Copy is required here because leaveMulticastGroup will erase the iterator
        const auto group = *_multicastGroups.begin();
        if(!leaveMulticastGroup(group))
            allSuccess = false;
    }
    return allSuccess;
}

bool Socket::isMulticastGroupPresent(const QString& groupAddress)
{
    return _multicastGroups.find(groupAddress) != _multicastGroups.end();
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

std::unique_ptr<Worker> Socket::createWorker() { return std::make_unique<Worker>(); }

std::shared_ptr<Datagram> Socket::makeDatagram(const size_t length) { return _cache.make(length); }

bool Socket::sendDatagram(const uint8_t* buffer, const size_t length, const QString& address,
    const uint16_t port, const uint8_t ttl)
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

bool Socket::sendDatagram(const char* buffer, const size_t length, const QString& address,
    const uint16_t port, const uint8_t ttl)
{
    return sendDatagram(reinterpret_cast<const uint8_t*>(buffer), length, address, port, ttl);
}

bool Socket::sendDatagram(std::shared_ptr<Datagram> datagram, const QString& address,
    const uint16_t port, const uint8_t ttl)
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

    if(datagram->ttl == 0)
    {
        LOG_ERR("Error: Fail to send datagram because the Ttl is 0");
        return false;
    }

    Q_EMIT sendDatagramToWorker(std::move(datagram));

    return true;
}

void Socket::onDatagramReceived(const SharedDatagram& datagram)
{
    Q_CHECK_PTR(datagram.get());
    Q_EMIT datagramReceived(datagram);
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
