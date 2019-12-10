// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header

// Dependencies Header

// Application Header
#include <NetUdp/Server.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

NETUDP_USING_NAMESPACE;

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

bool Server::start()
{
    if (!AbstractServer::start())
        return false;

    Q_ASSERT(_worker.get() == nullptr);
    Q_ASSERT(_workerThread.get() == nullptr);

    _worker = createWorker();
    _workerThread = std::make_unique<QThread>();

    connect(_workerThread.get(), &QThread::finished,
        [this]() { _worker = nullptr; }
    );

    if (objectName().size())
        _workerThread->setObjectName(objectName() + " Worker");
    else
        _workerThread->setObjectName("Server Worker");
    _worker->moveToThread(_workerThread.get());

    _worker->_watchdogTimeout = watchdogPeriodMs();
    _worker->_address = address();
    _worker->_port = port();

    for (const auto& it : multicastGroupsSet())
        _worker->_multicastGroups.insert(it, false);

    _worker->_multicastInterfaceName = multicastInterfaceName();
    _worker->_multicastLoopback = multicastLoopback();

    connect(this, &Server::startWorker, _worker.get(), &ServerWorker::onStart);
    connect(this, &Server::stopWorker, _worker.get(), &ServerWorker::onStop);

    connect(this, &Server::joinMulticastGroupWorker, _worker.get(), &ServerWorker::joinMulticastGroup);
    connect(this, &Server::leaveMulticastGroupWorker, _worker.get(), &ServerWorker::leaveMulticastGroup);

    connect(this, &Server::addressChanged, _worker.get(), &ServerWorker::setAddress);
    connect(this, &Server::portChanged, _worker.get(), &ServerWorker::setPort);
    connect(this, &Server::multicastLoopbackChanged, _worker.get(), &ServerWorker::setMulticastLoopback);
    connect(this, &Server::multicastInterfaceNameChanged, _worker.get(), &ServerWorker::setMulticastInterfaceName);
    connect(this, &Server::watchdogPeriodMsChanged, _worker.get(), &ServerWorker::setWatchdogTimeout);

    connect(this, &Server::sendDatagramToWorker, _worker.get(), &ServerWorker::onSendDatagram);

    connect(_worker.get(), &ServerWorker::isBoundedChanged, this, &Server::onBoundedChanged);
    connect(_worker.get(), &ServerWorker::socketError, this, &Server::socketError);

    connect(_worker.get(), &ServerWorker::rxBytesCounterChanged, this, &Server::onWorkerRxPerSecondsChanged);
    connect(_worker.get(), &ServerWorker::txBytesCounterChanged, this, &Server::onWorkerTxPerSecondsChanged);

    _workerThread->start();

    Q_EMIT startWorker();

    return true;
}

bool Server::stop()
{
    if (!AbstractServer::stop())
        return false;

    Q_EMIT stopWorker();

    _workerThread->exit();
    _workerThread->wait();
    _workerThread = nullptr;

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

bool Server::sendDatagram(uint8_t* buffer, const size_t length, const QHostAddress& address, const uint16_t port,
    const uint8_t ttl)
{
    auto datagram = std::make_shared<NetUdp::Datagram>();
    const auto datagramLength = length;
    datagram->buffer = std::make_unique<uint8_t[]>(datagramLength);
    memcpy(datagram->buffer.get(), buffer, length);
    datagram->length = datagramLength;
    datagram->destination = address;
    datagram->port = port;
    datagram->ttl = ttl;

    return sendDatagram(datagram);
}

bool Server::sendDatagram(std::shared_ptr<Datagram> datagram)
{
    Q_EMIT sendDatagramToWorker(std::move(datagram));
    return true;
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
