// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Server.hpp>
#include <Net/Udp/ServerWorker.hpp>

// Qt Header
#include <QThread>
#include <QLoggingCategory>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(NETUDP_SERVER_LOGCAT, "net.udp.server")

using namespace Net::Udp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

Server::Server(QObject* parent): AbstractServer(parent)
{
}

Server::~Server()
{
    // ) We can't destroy a thread that is running
    if (_workerThread)
    {
        _workerThread->exit();
        _workerThread->wait();
    }
}

bool Server::inputEnabled() const
{
    return _inputEnabled;
}

void Server::setInputEnabled(const bool enabled)
{
    if (enabled != _inputEnabled)
    {
        _inputEnabled = enabled;
        Q_EMIT inputEnabledChanged(enabled);
    }
}

bool Server::useWorkerThread() const
{
    return _useWorkerThread;
}

void Server::setUseWorkerThread(const bool enabled)
{
    if (enabled != _useWorkerThread)
    {
        _useWorkerThread = enabled;
        Q_EMIT useWorkerThreadChanged(enabled);

        stop();
        start();
    }
}

bool Server::start()
{
    if (!AbstractServer::start())
        return false;

    Q_ASSERT(_worker.get() == nullptr);
    Q_ASSERT(_workerThread.get() == nullptr);

    _worker = createWorker();
    if(_useWorkerThread)
    {
        _workerThread = std::make_unique<QThread>();

        connect(_workerThread.get(), &QThread::finished,
            [this]() { _worker = nullptr; }
        );

        if (objectName().size())
            _workerThread->setObjectName(objectName() + " Worker");
        else
            _workerThread->setObjectName("Server Worker");
        _worker->moveToThread(_workerThread.get());        
    }

    _worker->_watchdogTimeout = watchdogPeriodMs();
    _worker->_rxAddress = rxAddress();
    _worker->_rxPort = rxPort();
    _worker->_txPort = txPort();

    for (const auto& it : multicastGroupsSet())
        _worker->_multicastGroups.insert(it, false);

    _worker->_multicastInterface = QNetworkInterface::interfaceFromName(multicastInterfaceName());
    _worker->_multicastLoopback = multicastLoopback();
    _worker->_inputEnabled = inputEnabled();

    connect(this, &Server::startWorker, _worker.get(), &ServerWorker::onStart);
    connect(this, &Server::stopWorker, _worker.get(), &ServerWorker::onStop);
    connect(this, &Server::restartWorker, _worker.get(), &ServerWorker::onRestart);

    connect(this, &Server::joinMulticastGroupWorker, _worker.get(), &ServerWorker::joinMulticastGroup);
    connect(this, &Server::leaveMulticastGroupWorker, _worker.get(), &ServerWorker::leaveMulticastGroup);

    connect(this, &Server::rxAddressChanged, _worker.get(), &ServerWorker::setAddress);
    connect(this, &Server::rxPortChanged, _worker.get(), &ServerWorker::setRxPort);
    connect(this, &Server::txPortChanged, _worker.get(), &ServerWorker::setTxPort);
    connect(this, &Server::separateRxTxSocketsChanged, _worker.get(), &ServerWorker::setSeparateRxTxSockets);
    connect(this, &Server::multicastLoopbackChanged, _worker.get(), &ServerWorker::setMulticastLoopback);
    connect(this, &Server::multicastInterfaceNameChanged, _worker.get(), &ServerWorker::setMulticastInterfaceName);
    connect(this, &Server::inputEnabledChanged, _worker.get(), &ServerWorker::setInputEnabled);
    connect(this, &Server::watchdogPeriodMsChanged, _worker.get(), &ServerWorker::setWatchdogTimeout);

    connect(this, &Server::sendDatagramToWorker, _worker.get(), &ServerWorker::onSendDatagram);
    connect(_worker.get(), &ServerWorker::receivedDatagram, this, &Server::onDatagramReceived);

    connect(_worker.get(), &ServerWorker::isBoundedChanged, this, &Server::onBoundedChanged);
    connect(_worker.get(), &ServerWorker::socketError, this, &Server::socketError);

    connect(_worker.get(), &ServerWorker::rxBytesCounterChanged, this, &Server::onWorkerRxPerSecondsChanged);
    connect(_worker.get(), &ServerWorker::txBytesCounterChanged, this, &Server::onWorkerTxPerSecondsChanged);
    connect(_worker.get(), &ServerWorker::rxPacketsCounterChanged, this, &Server::onWorkerPacketsRxPerSecondsChanged);
    connect(_worker.get(), &ServerWorker::txPacketsCounterChanged, this, &Server::onWorkerPacketsTxPerSecondsChanged);

    if (_useWorkerThread)
        _workerThread->start();

    Q_EMIT startWorker();

    return true;
}

bool Server::stop()
{
    if (!AbstractServer::stop())
        return false;

    _cache.clear();

    Q_EMIT stopWorker();

    disconnect(_worker.get());
    _worker.get()->disconnect();

    setRxBytesPerSeconds(0);
    setTxBytesPerSeconds(0);

    setRxPacketsPerSeconds(0);
    setTxPacketsPerSeconds(0);

    if(_workerThread)
    {
        _workerThread->exit();
        _workerThread->wait();
        _workerThread = nullptr;        
    }
    else
    {
        _worker->deleteLater();
        _worker.release();
    }

    return true;
}

bool Server::restart()
{
    Q_EMIT restartWorker();
    return true;
}

bool Server::joinMulticastGroup(const QString& groupAddress)
{
    if (AbstractServer::joinMulticastGroup(groupAddress))
    {
        Q_EMIT joinMulticastGroupWorker(groupAddress);
        return true;
    }
    return false;
}

bool Server::leaveMulticastGroup(const QString& groupAddress)
{
    if (AbstractServer::leaveMulticastGroup(groupAddress))
    {
        Q_EMIT leaveMulticastGroupWorker(groupAddress);
        return true;
    }
    return false;
}

std::unique_ptr<ServerWorker> Server::createWorker() const
{
    return std::make_unique<ServerWorker>();
}

std::shared_ptr<RecycledDatagram> Server::makeDatagram(const size_t length)
{
    return _cache.make(length);
}

bool Server::sendDatagram(uint8_t* buffer, const size_t length, const QHostAddress& address, const uint16_t port,
                          const uint8_t ttl)
{
    if (!isRunning() && !isBounded())
    {
        if (!isRunning())
            qCDebug(NETUDP_SERVER_LOGCAT, "Error: Fail to send datagram because the Udp Server isn't running");
        else if (!isBounded())
            qCDebug(NETUDP_SERVER_LOGCAT, "Error: Fail to send datagram because the Udp Server isn't bounded to any interfaces.");
        return false;
    }

    if (length <= 0)
    {
        qCDebug(NETUDP_SERVER_LOGCAT, "Error: Fail to send datagram because the length is <= 0");
        return false;
    }

    if (ttl == 0)
    {
        qCDebug(NETUDP_SERVER_LOGCAT, "Error: Fail to send datagram because the Ttl is 0");
        return false;
    }

    auto datagram = _cache.make(length);
    memcpy(datagram->buffer(), buffer, length);
    datagram->destinationAddress = address;
    datagram->destinationPort = port;
    datagram->ttl = ttl;

    Q_EMIT sendDatagramToWorker(std::move(datagram));

    return true;
}

bool Server::sendDatagram(std::shared_ptr<RecycledDatagram> datagram)
{
    if (!isRunning() && !isBounded())
    {
        if (!isRunning())
            qCDebug(NETUDP_SERVER_LOGCAT, "Error: Fail to send datagram because the Udp Server isn't running");
        else if (!isBounded())
            qCDebug(NETUDP_SERVER_LOGCAT, "Error: Fail to send datagram because the Udp Server isn't bounded to any interfaces.");
        return false;
    }

    if (datagram->length() <= 0)
    {
        qCDebug(NETUDP_SERVER_LOGCAT, "Error: Fail to send datagram because the length is <= 0");
        return false;
    }

    if (datagram->ttl == 0)
    {
        qCDebug(NETUDP_SERVER_LOGCAT, "Error: Fail to send datagram because the Ttl is 0");
        return false;
    }

    Q_EMIT sendDatagramToWorker(std::move(datagram));

    return true;
}

void Server::onDatagramReceived(const SharedDatagram datagram)
{
    Q_CHECK_PTR(datagram.get());
}

void Server::onBoundedChanged(const bool isBounded)
{
    setIsBounded(isBounded);
}

void Server::onWorkerRxPerSecondsChanged(const quint64 rxBytes)
{
    setRxBytesPerSeconds(rxBytes);
    setRxBytesTotal(rxBytesTotal() + rxBytes);
}

void Server::onWorkerTxPerSecondsChanged(const quint64 txBytes)
{
    setTxBytesPerSeconds(txBytes);
    setTxBytesTotal(txBytesTotal() + txBytes);
}

void Server::onWorkerPacketsRxPerSecondsChanged(const quint64 rxPackets)
{
    setRxPacketsPerSeconds(rxPackets);
    setRxPacketsTotal(rxPacketsTotal() + rxPackets);
}

void Server::onWorkerPacketsTxPerSecondsChanged(const quint64 txPackets)
{
    setTxPacketsPerSeconds(txPackets);
    setTxPacketsTotal(txPacketsTotal() + txPackets);
}
