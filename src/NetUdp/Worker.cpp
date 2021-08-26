// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <NetUdp/Worker.hpp>
#include <NetUdp/Logger.hpp>
#include <NetUdp/InterfacesProvider.hpp>
#include <NetUdp/RecycledDatagram.hpp>
#include <QtCore/QTimer>
#include <QtCore/QElapsedTimer>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkDatagram>
#include <Recycler/Circular.hpp>
#include <algorithm>

namespace netudp {

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

static const quint64 disableSocketTimeout = 10000;

struct WorkerPrivate
{
    using MulticastGroupList = std::set<QString>;
    using MulticastInterfaceList = std::set<QString>;
    using InterfaceToMulticastSocket = std::map<QString, QUdpSocket*>;

    // ──────── ATTRIBUTE ────────
    QUdpSocket* socket = nullptr;
    QUdpSocket* rxSocket = nullptr;
    QTimer* watchdog = nullptr;
    recycler::Circular<RecycledDatagram> cache;
    bool isBounded = false;
    quint64 watchdogTimeout = 5000;
    QString rxAddress;
    quint16 rxPort = 0;
    quint16 txPort = 0;

    // ─── Multicast - Input ───

    // List of all multicast group the socket is listening to
    MulticastGroupList multicastGroups;

    // Incoming interfaces for multicast packets that are going to be listened.
    // If empty, then every interfaces available on your system that support multicast are going to be listened.
    // This set doesn't reflect the interfaces that are really joined. If you want to know which interfaces are really joined
    // or which one failed to joined, then check `_failedToJoinIfaces` & '_joinedIfaces'
    MulticastInterfaceList incomingMulticastInterfaces;
    MulticastInterfaceList allMulticastInterfaces;

    MulticastInterfaceList& currentMulticastInterfaces()
    {
        return incomingMulticastInterfaces.empty() ? allMulticastInterfaces : incomingMulticastInterfaces;
    }

    // Keep track of all multicast interfaces, to know which one are available
    //std::set<QString> allMulticastInterfaces;

    // Keep track of group that were joined on each interfaces
    std::map<QString, MulticastGroupList> joinedMulticastGroups;

    // Keep track of the interfaces that couldn't be joined, to retry periodically when the interface will be up again
    // See 'startListeningMulticastInterfaceWatcher'
    std::map<QString, MulticastGroupList> failedJoiningMulticastGroup;

    // ─── Multicast - Output ───

    // User requested outgoing multicast interfaces
    MulticastInterfaceList outgoingMulticastInterfaces;

    // Keep track of interfaces for which sockets couldn't be created and try later.
    // See 'startOutputMulticastInterfaceWatcher'
    MulticastInterfaceList failedToInstantiateMulticastTxSockets;

    // Create a socket for each interface on which we want to multicast
    // This is way more efficient than changing IP_MULTICAST_IF at each packet send.
    // socket won't be used for multicast packet sending
    // Port used for each socket will be the one specified by txPort
    InterfaceToMulticastSocket multicastTxSockets;

    // Indicate if multicastSockets are created or not.
    // !_p->multicastSockets.empty() can't be used since it's possible that they is no interface at all.
    // In that case the regular socket should be used to send datagram.
    // Creation and destruction is controlled by startMulticastOutputSockets/stopMulticastOutputSockets.
    // Each time multicastSocketsInstantiated to true, a QElapsedTimer is instantiated.
    // If no multicast datagram are send for 30s, then all sockets are destroyed.²
    bool multicastTxSocketsInstantiated = false;

    bool multicastLoopback = false;
    quint8 multicastTtl = 0;
    bool inputEnabled = false;
    bool separateRxTxSockets = false;

    // ──────── MULTICAST INTERFACE JOIN WATCHER ────────
    // Created when at least one interface is being joined. ie (!_p->joinedMulticastGroups.empty() || !_p->failedJoiningMulticastGroup.empty())
    // Destroy in 'onStop', when joinedMulticastGroups & failedJoiningMulticastGroup are both empty
    QTimer* listeningMulticastInterfaceWatcher = nullptr;

    void startListeningMulticastInterfaceWatcher();
    void stopListeningMulticastInterfaceWatcher();

    // ──────── MULTICAST TX JOIN WATCHER ────────
    // Created when
    QTimer* outputMulticastInterfaceWatcher = nullptr;
    QElapsedTimer txMulticastPacketElapsedTime;

    quint64 rxBytesCounter = 0;
    quint64 txBytesCounter = 0;
    quint64 rxPacketsCounter = 0;
    quint64 txPacketsCounter = 0;
    quint64 rxInvalidPacket = 0;
    QTimer* bytesCounterTimer = nullptr;
};

Worker::Worker(QObject* parent)
    : QObject(parent)
    , _p(std::make_unique<WorkerPrivate>())
{
    LOG_DEV_DEBUG("Constructor");
}

Worker::~Worker()
{
    LOG_DEV_DEBUG("Destructor");
}

bool Worker::isBounded() const
{
    return _p->isBounded;
}

quint64 Worker::watchdogTimeout() const
{
    return _p->watchdogTimeout;
}

QString Worker::rxAddress() const
{
    return _p->rxAddress;
}

quint16 Worker::rxPort() const
{
    return _p->rxPort;
}

quint16 Worker::txPort() const
{
    return _p->txPort;
}

bool Worker::multicastLoopback() const
{
    return _p->multicastLoopback;
}

quint8 Worker::multicastTtl() const
{
    return _p->multicastTtl;
}

bool Worker::inputEnabled() const
{
    return _p->inputEnabled;
}

bool Worker::separateRxTxSocketsChanged() const
{
    return _p->separateRxTxSockets;
}

QUdpSocket* Worker::rxSocket() const
{
    if(_p->separateRxTxSockets)
        return _p->rxSocket;
    return _p->socket;
}

size_t Worker::cacheSize() const
{
    return _p->cache.size();
}

bool Worker::resizeCache(size_t length)
{
    return _p->cache.resize(length);
}

void Worker::clearCache()
{
    _p->cache.clear();
}

void Worker::releaseCache()
{
    _p->cache.release();
}

std::shared_ptr<Datagram> Worker::makeDatagram(const size_t length)
{
    return _p->cache.make(length);
}

void Worker::onRestart()
{
    onStop();
    onStart();
}

void Worker::onStart()
{
    if(_p->socket)
    {
        LOG_DEV_ERR("Can't start udp socket worker because socket is already valid");
        return;
    }

    if(_p->inputEnabled)
    {
        if(_p->rxAddress.isEmpty())
            LOG_DEV_INFO("Start Udp Socket Worker rx : {}:{}, tx port : {}", _p->rxAddress.toStdString(), _p->rxPort, _p->txPort);
        else
            LOG_DEV_INFO("Start Udp Socket Worker rx port : {}, tx port : {}", _p->rxPort, _p->txPort);
    }
    else
    {
        if(_p->rxAddress.isEmpty())
            LOG_DEV_INFO("Start Udp Socket Worker rx : {}:{}", _p->rxAddress.toStdString(), _p->rxPort);
        else
            LOG_DEV_INFO("Start Udp Socket Worker rx port : {}", _p->rxPort);
    }

    _p->isBounded = false;
    Q_ASSERT(_p->watchdog == nullptr);

    _p->failedJoiningMulticastGroup.clear();
    _p->joinedMulticastGroups.clear();
    _p->allMulticastInterfaces.clear();

    // Create the socket (and a second one for rx if required)
    if(_p->socket)
        _p->socket->deleteLater();
    _p->socket = new QUdpSocket(this);
    const bool useTwoSockets = (_p->separateRxTxSockets || _p->txPort) && _p->inputEnabled;
    if(useTwoSockets)
    {
        LOG_DEV_INFO("Create separate rx socket");
        if(_p->rxSocket)
            _p->rxSocket->deleteLater();
        _p->rxSocket = new QUdpSocket(this);
    }

    connect(
        this,
        &Worker::queueStartWatchdog,
        this,
        [this]()
        {
            if(!_p->watchdog)
            {
                onStop();

                Q_ASSERT(!_p->watchdog);

                // ) Create a watchdog timer
                _p->watchdog = new QTimer(this);
                _p->watchdog->setTimerType(Qt::TimerType::VeryCoarseTimer);

                // ) Connect timeout
                connect(
                    _p->watchdog,
                    &QTimer::timeout,
                    this,
                    [this]()
                    {
                        LOG_DEV_INFO("Watchdog timeout, try to restart socket");
                        onRestart();
                    },
                    Qt::ConnectionType::QueuedConnection);

                LOG_INFO("Start watchdog to restart socket in {} millis", watchdogTimeout());
                // Start the watchdog
                _p->watchdog->start(watchdogTimeout());
            }
            else
            {
                LOG_DEV_WARN("Watchdog already running, fail to start it");
            }
        },
        Qt::QueuedConnection);

    // Connect to socket signals
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(_p->socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Worker::onSocketError);
#else
    connect(_p->socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &Worker::onSocketError);
#endif

    if(rxSocket())
    {
        connect(rxSocket(), &QUdpSocket::readyRead, this, &Worker::readPendingDatagrams);
        // _p->socket is always bounded because binded to QHostAddress(). Only rxSocket can go wrong
        connect(rxSocket(), &QUdpSocket::stateChanged, this, &Worker::onSocketStateChanged);
    }

    if(useTwoSockets)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        connect(_p->rxSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Worker::onRxSocketError);
#else
        connect(_p->rxSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &Worker::onRxSocketError);
#endif
    }

    // Bind to socket
    // When only in output mode, calling _p->socket->bind() doesn't allow to set multicast ttl.
    // But calling _p->socket->bind(QHostAddress()) allow to set the ttl.
    // From what i understand, _p->socket->bind() will call _p->socket->bind(QHostAddress::Any) internally.
    // _p->socket->bind(QHostAddress()) bind to a non valid host address and random port will be choose.
    // Qt multicast issues are non resolved ? https://forum.qt.io/topic/78090/multicast-issue-possible-bug/17
    const auto bindSuccess = [&]()
    {
        if(useTwoSockets)
            return _p->rxSocket->bind(QHostAddress(_p->rxAddress),
                       _p->rxPort,
                       QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)
                   && _p->socket->bind(QHostAddress(), _p->txPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

        if(_p->inputEnabled)
            return _p->socket->bind(QHostAddress(_p->rxAddress),
                _p->rxPort,
                QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
        return _p->socket->bind(QHostAddress(), _p->txPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    }();

    // Finish the start of the socket
    // Or start watchdog on failure
    if(bindSuccess)
    {
        LOG_INFO("Success bind to {}:{}", qPrintable(_p->socket->localAddress().toString()), _p->socket->localPort());

        if(!_p->multicastGroups.empty() && rxSocket() && _p->inputEnabled)
        {
            // Join multicast groups either on every interfaces or in '_p->incomingMulticastInterfaces' given by user
            if(_p->incomingMulticastInterfaces.empty())
            {
                tryJoinAllAvailableInterfaces();
            }
            else
            {
                for(const auto& interfaceName: _p->incomingMulticastInterfaces)
                {
                    for(const auto& group: _p->multicastGroups)
                        joinAndTrackMulticastGroup(group, interfaceName);
                }
            }
        }

        setMulticastLoopbackToSocket();
        startBytesCounter();
    }
    else
    {
        LOG_ERR("Fail to bind to {} : {}", qPrintable(_p->rxAddress.isEmpty() ? "Any" : _p->rxAddress), _p->rxPort);
        //_p->socket = nullptr;
        //_p->rxSocket = nullptr;
        startWatchdog();
    }
}

void Worker::onStop()
{
    // Important watchdog can be valid while socket is not !!
    stopWatchdog();

    if(!_p->socket)
    {
        Q_ASSERT(rxSocket() == nullptr);
        Q_ASSERT(_p->multicastTxSockets.empty());
        Q_ASSERT(_p->watchdog == nullptr);
        Q_ASSERT(!_p->isBounded);
        LOG_DEV_WARN("Can't stop udp socket worker because socket isn't valid");
        return;
    }

    LOG_INFO("Stop Udp Socket Worker");

    stopListeningMulticastInterfaceWatcher();
    stopOutputMulticastInterfaceWatcher();
    stopBytesCounter();
    disconnect(this, nullptr, this, nullptr);

    if(_p->socket)
    {
        disconnect(_p->socket, nullptr, this, nullptr);
        _p->socket->deleteLater();
    }
    _p->socket = nullptr;
    _p->isBounded = false;
    Q_EMIT isBoundedChanged(_p->isBounded);
    if(_p->rxSocket)
    {
        disconnect(_p->rxSocket, nullptr, this, nullptr);
        _p->rxSocket->deleteLater();
    }
    _p->rxSocket = nullptr;
    _p->multicastTtl = 0;

    // Delete every multicast outgoing socket
    for(const auto& [iface, socket]: _p->multicastTxSockets)
        socket->deleteLater();
    _p->multicastTxSockets.clear();

    _p->failedJoiningMulticastGroup.clear();
    _p->joinedMulticastGroups.clear();
    _p->allMulticastInterfaces.clear();
    _p->failedToInstantiateMulticastTxSockets.clear();
    _p->multicastTxSocketsInstantiated = false;
}

void Worker::initialize(quint64 watchdog,
    QString rxAddress,
    quint16 rxPort,
    quint16 txPort,
    bool separateRxTxSocket,
    const std::set<QString>& multicastGroup,
    const std::set<QString>& multicastListeningInterfaces,
    const std::set<QString>& multicastOutgoingInterfaces,
    bool inputEnabled,
    bool multicastLoopback)
{
    _p->watchdogTimeout = watchdog;
    _p->rxAddress = rxAddress;
    _p->rxPort = rxPort;
    _p->txPort = txPort;
    _p->separateRxTxSockets = separateRxTxSocket;
    _p->multicastGroups = multicastGroup;
    _p->incomingMulticastInterfaces = multicastListeningInterfaces;
    _p->outgoingMulticastInterfaces = multicastOutgoingInterfaces;
    _p->inputEnabled = inputEnabled;
    _p->multicastLoopback = multicastLoopback;
}

void Worker::setWatchdogTimeout(const quint64 ms)
{
    if(_p->watchdogTimeout != ms)
        _p->watchdogTimeout = ms;
}

void Worker::setAddress(const QString& address)
{
    if(address != _p->rxAddress)
    {
        _p->rxAddress = address;
        onRestart();
    }
}

void Worker::setRxPort(const quint16 port)
{
    if(port != _p->rxPort)
    {
        _p->rxPort = port;
        if(_p->inputEnabled)
            onRestart();
    }
}

void Worker::setInputEnabled(const bool enabled)
{
    LOG_DEV_INFO("Set Input Enabled : {}", enabled);
    if(enabled != _p->inputEnabled)
    {
        _p->inputEnabled = enabled;
        onRestart();
    }
}

void Worker::setTxPort(const quint16 port)
{
    if(port != _p->txPort)
    {
        _p->txPort = port;
        onRestart();
    }
}

void Worker::joinMulticastGroup(const QString& address)
{
    LOG_INFO("Join Multicast group {}", address.toStdString());

    const auto [groupIt, groupInsertSuccess] = _p->multicastGroups.insert(address);

    // Group is already joined
    if(!groupInsertSuccess)
        return;

    // Input disable, we should listen to anything nor subscribe
    if(!_p->inputEnabled)
        return;

    // No rx socket mean that the socket isn't started. 'onStart' will take care of really joining the group.
    if(!rxSocket())
        return;

    if(_p->incomingMulticastInterfaces.empty())
    {
        if(_p->allMulticastInterfaces.empty())
        {
            tryJoinAllAvailableInterfaces();
        }
        else
        {
            // Join the group address an each interface an keep track of success or fail
            for(const auto& interfaceName: _p->allMulticastInterfaces)
                joinAndTrackMulticastGroup(address, interfaceName);
        }
    }
    else
    {
        // Join the group address an each interface an keep track of success or fail
        for(const auto& interfaceName: _p->incomingMulticastInterfaces)
            joinAndTrackMulticastGroup(address, interfaceName);
    }
}

void Worker::leaveMulticastGroup(const QString& address)
{
    LOG_INFO("Leave Multicast group {}", address.toStdString());

    const auto it = _p->multicastGroups.find(address);

    // Group isn't present
    if(it == _p->multicastGroups.end())
        return;

    _p->multicastGroups.erase(it);

    // No rx socket mean that the socket isn't started. It mean that no multicast group are also joined
    if(!rxSocket())
        return;

    std::vector<QString> interfaceNameJoinedToRemove;
    std::vector<QString> interfaceNameFailedToRemove;

    // Leave the multicast group on every interface that were joined successfully
    for(auto& [interfaceName, groups]: _p->joinedMulticastGroups)
    {
        // Really leave if the address was erased(ie found) in the set.
        if(groups.erase(address))
            socketLeaveMulticastGroup(address, interfaceName);

        if(groups.empty())
            interfaceNameJoinedToRemove.push_back(interfaceName);
    }

    // And also clean _p->failedJoiningMulticastGroup from the multicast group we are leaving
    for(auto& [interfaceName, groups]: _p->failedJoiningMulticastGroup)
    {
        groups.erase(address);

        if(groups.empty())
            interfaceNameFailedToRemove.push_back(interfaceName);
    }

    // Remove interfaces from that that have an empty key
    if(interfaceNameJoinedToRemove.size() == _p->joinedMulticastGroups.size())
    {
        _p->joinedMulticastGroups.clear();
    }
    else
    {
        for(const auto& name: interfaceNameJoinedToRemove)
            _p->joinedMulticastGroups.erase(name);
    }

    if(interfaceNameFailedToRemove.size() == _p->failedJoiningMulticastGroup.size())
    {
        _p->failedJoiningMulticastGroup.clear();
    }
    else
    {
        for(const auto& name: interfaceNameFailedToRemove)
            _p->failedJoiningMulticastGroup.erase(name);
    }

    if(_p->multicastGroups.empty())
        stopListeningMulticastInterfaceWatcher();
}

void Worker::joinMulticastInterface(const QString& interfaceName)
{
    if(_p->incomingMulticastInterfaces.empty())
        tryLeaveAllAvailableInterfaces();

    const auto [interfaceNameIt, interfaceNameInsertSuccess] = _p->incomingMulticastInterfaces.insert(interfaceName);
    const auto interface = InterfacesProvider::interfaceFromName(interfaceName);

    // Interface already exist. We don't need to do any actions
    if(!interfaceNameInsertSuccess)
        return;

    // Input disable, don't need to do anything
    if(!_p->inputEnabled)
        return;

    // No rx socket mean that the socket isn't started. 'onStart' will take care of really joining the group.
    if(!rxSocket())
        return;

    // Join each multicast group
    for(const auto& group: _p->multicastGroups)
        joinAndTrackMulticastGroup(group, interfaceName);
}

void Worker::leaveMulticastInterface(const QString& interfaceName)
{
    // If the interface wasn't present, then don't need to do anything
    if(!_p->incomingMulticastInterfaces.erase(interfaceName))
        return;

    // No rx socket mean that the socket isn't started. It mean that no multicast group are also joined
    if(!rxSocket())
        return;

    // Leave all group join on interface 'interfaceName'
    const auto it = _p->joinedMulticastGroups.find(interfaceName);
    if(it != _p->joinedMulticastGroups.end())
    {
        const auto& groupJoined = it->second;
        for(const auto& group: groupJoined)
            socketLeaveMulticastGroup(group, interfaceName);

        _p->joinedMulticastGroups.erase(it);
    }

    // Clear cache of failed to join multicast group
    _p->failedJoiningMulticastGroup.erase(interfaceName);

    // Try to listen on every interfaces if _p->incomingMulticastInterfaces is empty
    if(_p->incomingMulticastInterfaces.empty())
    {
        // Order matter because 'tryJoinAllAvailableInterfaces' might recreate the timer
        stopListeningMulticastInterfaceWatcher();
        tryJoinAllAvailableInterfaces();
    }
}

void Worker::setMulticastLoopback(const bool loopback)
{
    LOG_DEV_INFO("Set Multicast Loopback : {}", loopback);

    if(_p->multicastLoopback != loopback)
    {
        _p->multicastLoopback = loopback;
        setMulticastLoopbackToSocket();
    }
}

void Worker::setMulticastOutgoingInterfaces(const QStringList& interfaces)
{
    // Nothing changed
    if(interfaces.empty() && _p->outgoingMulticastInterfaces.empty())
        return;

    // It will maybe be time to create new multicast socket
    if(_p->outgoingMulticastInterfaces.empty())
        destroyMulticastOutputSockets();

    // Copy interfaces to _p->outgoingMulticastInterfaces
    _p->outgoingMulticastInterfaces = WorkerPrivate::MulticastInterfaceList(interfaces.begin(), interfaces.end());

    // Destroy what was already instantiated.
    if(_p->multicastTxSocketsInstantiated)
        destroyMulticastOutputSockets();
}

void Worker::setSeparateRxTxSockets(const bool separateRxTxSocketsChanged)
{
    const bool shouldUseSeparate = separateRxTxSocketsChanged || _p->txPort;
    if(shouldUseSeparate != _p->separateRxTxSockets)
    {
        _p->separateRxTxSockets = shouldUseSeparate;
        if(_p->inputEnabled)
            onRestart();
    }
}

void Worker::tryJoinAllAvailableInterfaces()
{
    if(!_p->incomingMulticastInterfaces.empty())
        return;

    // Input disable, don't need to do anything
    if(!_p->inputEnabled)
        return;

    // No rx socket mean that the socket isn't started. 'onStart' will take care of really joining the group.
    if(!rxSocket())
        return;

    // Assert that model is really empty
    Q_ASSERT(_p->joinedMulticastGroups.empty());
    Q_ASSERT(_p->failedJoiningMulticastGroup.empty());

    // join every group on every interface
    // It is expected for interfaces to fail joining. Attempt to join will be made later.
    const auto& allInterfaces = InterfacesProvider::allInterfaces();
    for(const auto& interface: allInterfaces)
    {
        Q_ASSERT(interface);
        if(const auto [it, success] = _p->allMulticastInterfaces.insert(interface->name()); !success)
            continue;

        for(const auto& group: _p->multicastGroups)
        {
            joinAndTrackMulticastGroup(group, interface->name());
        }
    }
}

void Worker::tryLeaveAllAvailableInterfaces()
{
    if(!_p->incomingMulticastInterfaces.empty())
        return;

    // No rx socket mean that the socket isn't started. 'onStart' will take care of really joining the group.
    if(!rxSocket())
        return;

    for(const auto& [interfaceName, groups]: _p->joinedMulticastGroups)
    {
        for(const auto& group: groups)
            socketLeaveMulticastGroup(group, interfaceName);
    }

    _p->joinedMulticastGroups.clear();
    _p->failedJoiningMulticastGroup.clear();
    _p->allMulticastInterfaces.clear();
    stopListeningMulticastInterfaceWatcher();
}

bool Worker::joinAndTrackMulticastGroup(const QString& address, const QString& interfaceName)
{
    // Create a timer anyway, we have at least one interface to watch
    startListeningMulticastInterfaceWatcher();

    if(!socketJoinMulticastGroup(address, interfaceName))
    {
        _p->failedJoiningMulticastGroup[interfaceName].insert(address);
        return false;
    }

    const auto [insertedIt, groupInserted] = _p->joinedMulticastGroups[interfaceName].insert(address);

    // If assert here it mean something gone wrong in join/leave.
    Q_ASSERT(groupInserted);
    return true;
}

bool Worker::socketJoinMulticastGroup(const QString& address, const QString& interfaceName)
{
    const QHostAddress hostAddress(address);
    const auto networkInterface = QNetworkInterface::interfaceFromName(interfaceName);
    const auto networkInterfaceValid = networkInterface.isValid() && (networkInterface.flags() & QNetworkInterface::IsUp)
                                       && (networkInterface.flags() & QNetworkInterface::IsRunning)
                                       && ((networkInterface.flags() & QNetworkInterface::CanMulticast)
                                           || (multicastLoopback() && (networkInterface.flags() & QNetworkInterface::IsLoopBack)));

    // should be verified by Socket
    Q_ASSERT(hostAddress.isMulticast());
    Q_ASSERT(rxSocket());
    Q_ASSERT(_p->inputEnabled);

    if(!rxSocket())
    {
        LOG_ERR("Can't join multicast group when socket isn't started");
        return false;
    }

    // Try to join the multicast group
    if(!networkInterfaceValid || !rxSocket()->joinMulticastGroup(hostAddress, networkInterface))
    {
        if(networkInterfaceValid)
            LOG_ERR("Fail to join multicast group {} an interface {}. {}",
                address.toStdString(),
                interfaceName.toStdString(),
                rxSocket()->errorString().toStdString());
        else
            LOG_WARN("Fail to join multicast group {} an interface {} because interface is not valid. IsUp:{}, "
                     "IsRunning:{}, CanMulticast:{}, IsLoopBack:{}",
                address.toStdString(),
                interfaceName.toStdString(),
                bool(networkInterface.flags() & QNetworkInterface::IsUp),
                bool(networkInterface.flags() & QNetworkInterface::IsRunning),
                bool(networkInterface.flags() & QNetworkInterface::CanMulticast),
                bool(networkInterface.flags() & QNetworkInterface::IsLoopBack));
        return false;
    }

    LOG_INFO("Success Join multicast group {} on interface {}", address.toStdString(), interfaceName.toStdString());
    Q_EMIT multicastGroupJoined(address, interfaceName);

    return true;
}

bool Worker::socketLeaveMulticastGroup(const QString& address, const QString& interfaceName)
{
    const QHostAddress hostAddress(address);
    const auto networkInterface = QNetworkInterface::interfaceFromName(interfaceName);
    const auto networkInterfaceValid =
        networkInterface.isValid() && networkInterface.flags() & (QNetworkInterface::IsRunning | QNetworkInterface::IsUp);

    if(!networkInterfaceValid || !rxSocket()->leaveMulticastGroup(hostAddress, networkInterface))
    {
        if(networkInterfaceValid)
            LOG_ERR("Fail to leave multicast group {} an interface {}. {}",
                address.toStdString(),
                interfaceName.toStdString(),
                rxSocket()->errorString().toStdString());
        else
            LOG_ERR("Fail to leave multicast group {} an interface {} because interface is not valid",
                address.toStdString(),
                interfaceName.toStdString());
        return false;
    }

    LOG_INFO("Success Leave multicast group {} on interface {}", address.toStdString(), interfaceName.toStdString());
    Q_EMIT multicastGroupLeaved(address, interfaceName);

    return true;
}

void Worker::setMulticastLoopbackToSocket() const
{
    if(rxSocket() && _p->inputEnabled)
    {
        LOG_INFO("Set MulticastLoopbackOption to {}", int(_p->multicastLoopback));
        rxSocket()->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _p->multicastLoopback);
        if(rxSocket() != _p->socket)
            _p->socket->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _p->multicastLoopback);

        for(const auto& [interfaceName, socket]: _p->multicastTxSockets)
        {
            socket->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _p->multicastLoopback);
        }
    }
}

void Worker::startWatchdog()
{
    Q_EMIT queueStartWatchdog();
}

void Worker::stopWatchdog()
{
    if(_p->watchdog)
    {
        disconnect(_p->watchdog, nullptr, this, nullptr);
        _p->watchdog->stop();
        _p->watchdog->deleteLater();
        _p->watchdog = nullptr;
    }
}

void Worker::setMulticastTtl(const quint8 ttl)
{
    if(!_p->socket)
        return;

    if(!ttl)
        return;

    if(ttl != _p->multicastTtl)
    {
        // This should be set in case _p->multicastTxSockets is empty
        _p->multicastTtl = ttl;
        _p->socket->setSocketOption(QAbstractSocket::MulticastTtlOption, int(_p->multicastTtl ? _p->multicastTtl : 8));
        for(const auto& [interfaceName, socket]: _p->multicastTxSockets)
        {
            socket->setSocketOption(QAbstractSocket::SocketOption::MulticastTtlOption, int(_p->multicastTtl ? _p->multicastTtl : 8));
        };
    }
}

void Worker::startListeningMulticastInterfaceWatcher()
{
    if(!_p->listeningMulticastInterfaceWatcher)
    {
        _p->listeningMulticastInterfaceWatcher = new QTimer(this);

        _p->listeningMulticastInterfaceWatcher->setInterval(2500);
        _p->listeningMulticastInterfaceWatcher->setTimerType(Qt::VeryCoarseTimer);
        _p->listeningMulticastInterfaceWatcher->setSingleShot(false);

        connect(
            _p->listeningMulticastInterfaceWatcher,
            &QTimer::timeout,
            this,
            [this]()
            {
                // Should be deleted if _p->multicastGroups is empty
                Q_ASSERT(!_p->multicastGroups.empty());

                // Fetch all interfaces
                const auto allInterfaces = InterfacesProvider::allInterfaces();

                // if auto joining every interface
                if(_p->incomingMulticastInterfaces.empty())
                {
                    // Look for new interface to join. Every interface found are added to the '_p->failedJoiningMulticastGroup'.
                    // They will be joined with the other failed to join one.
                    for(const auto& interface: allInterfaces)
                    {
                        const auto interfaceName = interface->name();
                        if(_p->allMulticastInterfaces.find(interfaceName) == _p->allMulticastInterfaces.end())
                        {
                            LOG_DEV_INFO("Find a new interface to join for multicast : {}", interfaceName.toStdString());
                            _p->allMulticastInterfaces.insert(interfaceName);
                            _p->failedJoiningMulticastGroup.insert({interfaceName, _p->multicastGroups});
                        }
                    }

                    std::vector<QString> interfaceNameToRemove;

                    // Look for interfaces that disappear
                    for(const auto& interfaceName: _p->allMulticastInterfaces)
                    {
                        bool found = false;

                        // Search if interface is present
                        for(const auto& interface: allInterfaces)
                        {
                            if(interface->name() == interfaceName)
                            {
                                found = true;
                                break;
                            }
                        }

                        // If not add it to the list to delete later
                        if(!found)
                        {
                            LOG_DEV_INFO("Interface {} disappeared, It will be removed from tracked interfaces",
                                interfaceName.toStdString());
                            interfaceNameToRemove.push_back(interfaceName);
                        }
                    }

                    // And remove them
                    for(const auto& interfaceName: interfaceNameToRemove)
                    {
                        const auto& interfaceJoinedIt = _p->joinedMulticastGroups.find(interfaceName);
                        if(interfaceJoinedIt != _p->joinedMulticastGroups.end())
                        {
                            for(const auto& group: interfaceJoinedIt->second)
                            {
                                socketLeaveMulticastGroup(group, interfaceName);
                            }

                            _p->joinedMulticastGroups.erase(interfaceJoinedIt);
                        }
                        _p->failedJoiningMulticastGroup.erase(interfaceName);
                        _p->allMulticastInterfaces.erase(interfaceName);
                    }
                }

                std::vector<QString> disconnectedInterfaceList;

                // Look for interface disconnection
                for(const auto& [interfaceName, groups]: _p->joinedMulticastGroups)
                {
                    // Only try to reconnect to interface that seems valid
                    if(const auto& interface = InterfacesProvider::interfaceFromName(interfaceName))
                    {
                        const bool interfaceValid = interface->isValid() && interface->isRunning() && interface->isUp()
                                                    && (interface->canMulticast() || (multicastLoopback() && interface->isLoopBack()));
                        if(!interfaceValid)
                        {
                            LOG_DEV_INFO("Interface {} isn't valid anymore, It will be removed from joined interfaces. "
                                         "The worker will try to re join later the interface",
                                interfaceName.toStdString());
                            const auto& currentGroups = _p->failedJoiningMulticastGroup[interfaceName];

                            // Move the list of joined multicast group to the failed one.
                            if(currentGroups.empty())
                            {
                                _p->failedJoiningMulticastGroup[interfaceName] = groups;
                            }
                            else
                            {
                                WorkerPrivate::MulticastGroupList newList;
                                std::merge(currentGroups.begin(),
                                    currentGroups.end(),
                                    groups.begin(),
                                    groups.end(),
                                    std::inserter(newList, newList.begin()));
                                _p->failedJoiningMulticastGroup[interfaceName] = newList;
                            }

                            // And request a delete later (since we are iterating in this map)
                            disconnectedInterfaceList.push_back(interfaceName);
                        }
                    }
                }

                for(const auto& disconnectedInterfaceName: disconnectedInterfaceList)
                    _p->joinedMulticastGroups.erase(disconnectedInterfaceName);

                // Try to rejoin every groups
                for(auto& [interfaceName, groups]: _p->failedJoiningMulticastGroup)
                {
                    // Only try to reconnect to interface that seems valid
                    if(const auto& interface = InterfacesProvider::interfaceFromName(interfaceName))
                    {
                        const bool interfaceValid = interface->isValid() && interface->isRunning() && interface->isUp()
                                                    && (multicastLoopback() && interface->isLoopBack());
                        if(!interfaceValid)
                            continue;
                    }

                    std::vector<QString> successFullyJoinedGroup;

                    for(const auto& group: groups)
                    {
                        if(joinAndTrackMulticastGroup(group, interfaceName))
                            successFullyJoinedGroup.push_back(group);
                    }

                    for(const auto& group: successFullyJoinedGroup)
                    {
                        groups.erase(group);
                    }
                }

                // Remove interfaces with empty group
                for(auto it = _p->failedJoiningMulticastGroup.begin(); it != _p->failedJoiningMulticastGroup.end();)
                {
                    if(it->second.empty())
                        it = _p->failedJoiningMulticastGroup.erase(it);
                    else
                        ++it;
                }

                // Stop timer if nothing left to watch
                if(_p->joinedMulticastGroups.empty() || _p->failedJoiningMulticastGroup.empty())
                    stopListeningMulticastInterfaceWatcher();
            },
            Qt::QueuedConnection);

        _p->listeningMulticastInterfaceWatcher->start();
    }
}

void Worker::stopListeningMulticastInterfaceWatcher()
{
    if(_p->listeningMulticastInterfaceWatcher)
    {
        disconnect(_p->listeningMulticastInterfaceWatcher, nullptr, this, nullptr);
        _p->listeningMulticastInterfaceWatcher->deleteLater();
        _p->listeningMulticastInterfaceWatcher = nullptr;
    }
}

void Worker::startOutputMulticastInterfaceWatcher()
{
    if(!_p->outputMulticastInterfaceWatcher)
    {
        _p->outputMulticastInterfaceWatcher = new QTimer(this);
        _p->outputMulticastInterfaceWatcher->setInterval(2500);
        _p->outputMulticastInterfaceWatcher->setSingleShot(false);
        _p->outputMulticastInterfaceWatcher->setTimerType(Qt::VeryCoarseTimer);

        connect(_p->outputMulticastInterfaceWatcher,
            &QTimer::timeout,
            this,
            [this]()
            {
                // If too much time without datagram send, then we destroy every sockets
                if(_p->txMulticastPacketElapsedTime.elapsed() > disableSocketTimeout)
                {
                    destroyMulticastOutputSockets();
                    return;
                }

                const auto allInterfaces = InterfacesProvider::allInterfaces();

                // When instantiating sockets for all interfaces, check if new interfaces appeared
                if(_p->outgoingMulticastInterfaces.empty())
                {
                    // Try to find if new interfaces were created and try to join them
                    for(const auto& interface: allInterfaces)
                    {
                        Q_CHECK_PTR(interface);
                        const auto interfaceName = interface->name();
                        const auto multicastTxSocketFound = _p->multicastTxSockets.find(interfaceName) != _p->multicastTxSockets.end();
                        const auto multicastFailedToCreateFound = _p->failedToInstantiateMulticastTxSockets.find(interfaceName)
                                                                  != _p->failedToInstantiateMulticastTxSockets.end();

                        // New interface detected, It's added to the list of _p->failedToInstantiateMulticastTxSockets
                        // Then it will be instantiated by the step of the algorithm.
                        if(!multicastFailedToCreateFound && !multicastTxSocketFound)
                            _p->failedToInstantiateMulticastTxSockets.insert(interfaceName);
                    }
                }

                const auto isInterfacePresent = [&](const QString& interfaceName)
                {
                    for(const auto& interface: allInterfaces)
                    {
                        if(interface->name() == interfaceName)
                            return true;
                    }
                    return false;
                };

                // Check for interfaces that are no longer here
                for(auto it = _p->failedToInstantiateMulticastTxSockets.begin(); it != _p->failedToInstantiateMulticastTxSockets.end();)
                {
                    const auto interfaceName = *it;
                    if(isInterfacePresent(interfaceName))
                    {
                        ++it;
                    }
                    else
                    {
                        LOG_DEV_INFO("Detect interface {} disappear, stop trying to instantiate a udp multicast socket for it",
                            interfaceName.toStdString());
                        it = _p->failedToInstantiateMulticastTxSockets.erase(it);
                    }
                }
                for(auto it = _p->multicastTxSockets.begin(); it != _p->multicastTxSockets.end();)
                {
                    const auto [interfaceName, socket] = *it;
                    if(isInterfacePresent(interfaceName))
                    {
                        ++it;
                    }
                    else
                    {
                        LOG_DEV_INFO("Detect interface {} disappear, delete the associated multicast socket", interfaceName.toStdString());
                        socket->deleteLater();
                        it = _p->multicastTxSockets.erase(it);
                    }
                }

                const auto failedToInstantiateMulticastTxSocketsCopy = _p->failedToInstantiateMulticastTxSockets;
                _p->failedToInstantiateMulticastTxSockets.clear();

                // Then check all the socket that failed to be instantiated and try again.
                for(const auto& interfaceName: failedToInstantiateMulticastTxSocketsCopy)
                {
                    const auto interface = InterfacesProvider::interfaceFromName(interfaceName);
                    Q_CHECK_PTR(interface);
                    createMulticastSocketForInterface(*interface);
                }
            });

        _p->outputMulticastInterfaceWatcher->start();
    }
}

void Worker::stopOutputMulticastInterfaceWatcher()
{
    if(_p->outputMulticastInterfaceWatcher)
    {
        disconnect(_p->outputMulticastInterfaceWatcher, nullptr, this, nullptr);
        _p->outputMulticastInterfaceWatcher->deleteLater();
        _p->outputMulticastInterfaceWatcher = nullptr;
    }
}

void Worker::createMulticastSocketForInterface(const IInterface& interface)
{
    const auto successCreateSocket = [&]() -> bool
    {
        const auto interfaceName = interface.name();
        const bool isInterfaceValid = interface.isValid() && interface.isUp() && interface.isRunning()
                                      && (interface.canMulticast() || (multicastLoopback() && interface.isLoopBack()));

        if(!isInterfaceValid)
        {
            LOG_DEV_DEBUG("Can't create multicast socket for interface {} because it's not valid: isValid: {}, isUp: {}, "
                          "isRunning: {}, canMulticast: {}, isLoopback: {}, multicastLoopback: {}",
                interface.name().toStdString(),
                interface.isValid(),
                interface.isUp(),
                interface.isRunning(),
                interface.canMulticast(),
                interface.isLoopBack(),
                multicastLoopback());
            return false;
        }

        if(_p->multicastTxSockets.find(interfaceName) != _p->multicastTxSockets.end())
        {
            LOG_DEV_ERR("Multicast tx socket is already instantiated for interface {}. This might hide a bug in "
                        "the interface retrieving system",
                interfaceName.toStdString());
            return false;
        }

        auto socket = new QUdpSocket(this);
        if(!socket->bind(QHostAddress(), 0, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint))
        {
            socket->deleteLater();
            return false;
        }

        socket->setMulticastInterface(QNetworkInterface::interfaceFromName(interfaceName));
        socket->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _p->multicastLoopback);

        //const auto returnedInterface = socket->multicastInterface();
        //LOG_DEV_INFO("returnedInterface.isValid : {}", returnedInterface.isValid());
        //LOG_DEV_INFO("returnedInterface.name : {}", returnedInterface.name().toStdString());

        const auto onError = [interfaceName, socket, this](QAbstractSocket::SocketError error)
        {
            LOG_DEV_WARN("{}: Multicast tx error: {}", interfaceName.toStdString(), socket->errorString().toStdString());
            socket->deleteLater();
            _p->multicastTxSockets.erase(interfaceName);
            _p->failedToInstantiateMulticastTxSockets.insert(interfaceName);
        };

        // Connect to socket signals
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, onError, Qt::QueuedConnection);
#else
        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, onError, Qt::QueuedConnection);
#endif

        const auto [it, success] = _p->multicastTxSockets.insert({interfaceName, socket});

        // This have been checked before creating the socket if(_p->multicastTxSockets.find(interfaceName) != _p->multicastTxSockets.end())
        Q_ASSERT(success);

        LOG_DEV_INFO("Create multicast tx socket for interface {}", interfaceName.toStdString());

        return true;
    }();

    if(!successCreateSocket)
        _p->failedToInstantiateMulticastTxSockets.insert(interface.name());
};

void Worker::createMulticastOutputSockets()
{
    if(_p->multicastTxSocketsInstantiated)
    {
        LOG_DEV_WARN("Can't create Multicast output sockets because they are already created. You should call "
                     "'destroyMulticastOutputSockets' before.");
        return;
    }

    // If _p->multicastTxSocketsInstantiated is false, _p->multicastTxSockets HAVE TO be empty
    Q_ASSERT(_p->multicastTxSockets.empty());

    // Create sockets only for the interfaces specified by user or for every interfaces if nothing is specified
    if(_p->outgoingMulticastInterfaces.empty())
    {
        LOG_DEV_INFO("Create multicast tx sockets for all interfaces");
        const auto interfaces = InterfacesProvider::allInterfaces();
        for(const auto& interface: interfaces)
        {
            Q_CHECK_PTR(interface);
            createMulticastSocketForInterface(*interface);
        }
    }
    else
    {
        for(const auto& interfaceName: _p->outgoingMulticastInterfaces)
        {
            const auto interface = InterfacesProvider::interfaceFromName(interfaceName);
            Q_CHECK_PTR(interface);
            createMulticastSocketForInterface(*interface);
        }
    }

    startOutputMulticastInterfaceWatcher();
    _p->multicastTxSocketsInstantiated = true;
}

void Worker::destroyMulticastOutputSockets()
{
    LOG_DEV_INFO("Destroy all multicast tx sockets");
    for(const auto& [interfaceName, socket]: _p->multicastTxSockets)
    {
        Q_CHECK_PTR(socket);
        socket->deleteLater();
    }
    _p->multicastTxSockets.clear();
    _p->failedJoiningMulticastGroup.clear();
    stopOutputMulticastInterfaceWatcher();
    _p->multicastTxSocketsInstantiated = false;
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

    if(!_p->socket)
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
            QNetworkDatagram d(QByteArray(reinterpret_cast<const char*>(datagram->buffer()), int(datagram->length())),
                host,
                datagram->destinationPort);

            d.setHopLimit(datagram->ttl ? datagram->ttl : -1);
            return _p->socket->writeDatagram(d);
        }

        // If multi
        if(isMulticast)
        {
            if(!_p->multicastTxSocketsInstantiated)
                createMulticastOutputSockets();

            if(!_p->multicastTxSockets.empty())
            {
                bool byteWrittenInitialized = false;
                qint64 bytes = 0;
                for(const auto& [interfaceName, socket]: _p->multicastTxSockets)
                {
                    socket->setSocketOption(QAbstractSocket::MulticastTtlOption, int(_p->multicastTtl ? _p->multicastTtl : 8));
                    const auto currentBytesWritten = socket->writeDatagram(reinterpret_cast<const char*>(datagram->buffer()),
                        datagram->length(),
                        host,
                        datagram->destinationPort);

                    if(!byteWrittenInitialized)
                    {
                        byteWrittenInitialized = true;
                        bytes = currentBytesWritten;
                    }
                }

                // _timeSinceNoTxMulticastDatagram can't be null when _p->multicastTxSockets are instantiated
                _p->txMulticastPacketElapsedTime.start();
                return bytes;
            }
        }

        return _p->socket->writeDatagram(reinterpret_cast<const char*>(datagram->buffer()),
            datagram->length(),
            host,
            datagram->destinationPort);
    }();

    if(bytesWritten <= 0 || bytesWritten != datagram->length())
    {
        startWatchdog();

        if(bytesWritten <= 0)
            LOG_ERR("Fail to send datagram to {}:{}, 0 bytes written out of {}. Restart Socket. {}",
                datagram->destinationAddress.toStdString(),
                datagram->destinationPort,
                datagram->length(),
                _p->socket->errorString().toStdString());
        else
            LOG_ERR("Fail to send datagram, {}/{} bytes written. Restart Socket. {}",
                static_cast<long long>(bytesWritten),
                static_cast<long long>(datagram->length()),
                _p->socket->errorString().toStdString());
        return;
    }

    _p->txBytesCounter += bytesWritten;
    ++_p->txPacketsCounter;
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
            LOG_DEV_WARN("Receive datagram with size 0. This may means : \n"
                         "- That host is unreachable (receive an ICMP packet destination unreachable).\n"
                         "- Your OS doesn't support IGMP (if last packet sent was multicast). "
                         "On unix system you can check with netstat -g");
            ++_p->rxInvalidPacket;
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
            ++_p->rxInvalidPacket;
            startWatchdog();
            return;
        }

        if(datagram.data().size() <= 0)
        {
            LOG_ERR("Receive datagram with size {}. Restart Socket.", datagram.data().size());
            ++_p->rxInvalidPacket;
            startWatchdog();
            return;
        }

        if(!isPacketValid(reinterpret_cast<const uint8_t*>(datagram.data().constData()), datagram.data().size()))
        {
            LOG_WARN("Receive not valid application datagram. Simply discard the packet");
            ++_p->rxInvalidPacket;
            continue;
        }

        if(datagram.data().size() > 65535)
        {
            LOG_ERR("Receive a datagram with size of {}, that is too big for a datagram. Restart "
                    "Socket.",
                datagram.data().size());
            ++_p->rxInvalidPacket;
            startWatchdog();
            return;
        }

        SharedDatagram sharedDatagram = makeDatagram(datagram.data().size());
        memcpy(sharedDatagram.get()->buffer(), reinterpret_cast<const uint8_t*>(datagram.data().constData()), datagram.data().size());
        sharedDatagram->destinationAddress = datagram.destinationAddress().toString();
        if(datagram.destinationPort() >= 0)
            sharedDatagram->destinationPort = datagram.destinationPort();
        sharedDatagram->senderAddress = datagram.senderAddress().toString();
        if(datagram.senderPort() >= 0)
            sharedDatagram->senderPort = datagram.senderPort();
        if(datagram.hopLimit() >= 0)
            sharedDatagram->ttl = datagram.hopLimit();

        _p->rxBytesCounter += datagram.data().size();
        ++_p->rxPacketsCounter;

        onDatagramReceived(sharedDatagram);
    }
}

void Worker::onDatagramReceived(const SharedDatagram& datagram)
{
    Q_EMIT datagramReceived(datagram);
}

void Worker::onSocketError(QAbstractSocket::SocketError error)
{
    onSocketErrorCommon(error, _p->socket);
}

void Worker::onRxSocketError(QAbstractSocket::SocketError error)
{
    onSocketErrorCommon(error, _p->rxSocket);
}

void Worker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    LOG_DEV_INFO("Socket State Changed to {} ({})", socketStateToString(socketState).toStdString(), socketState);

    if((socketState == QAbstractSocket::SocketState::BoundState && !_p->isBounded)
        || (socketState != QAbstractSocket::SocketState::BoundState && _p->isBounded))
    {
        _p->isBounded = socketState == QAbstractSocket::SocketState::BoundState;
        Q_EMIT isBoundedChanged(_p->isBounded);
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
    case QAbstractSocket::UnconnectedState:
        return QStringLiteral("UnconnectedState");
    case QAbstractSocket::HostLookupState:
        return QStringLiteral("HostLookupState");
    case QAbstractSocket::ConnectingState:
        return QStringLiteral("ConnectingState");
    case QAbstractSocket::ConnectedState:
        return QStringLiteral("ConnectedState");
    case QAbstractSocket::BoundState:
        return QStringLiteral("BoundState");
    case QAbstractSocket::ListeningState:
        return QStringLiteral("ListeningState");
    case QAbstractSocket::ClosingState:
        return QStringLiteral("ClosingState");
    default:;
    }
    return QStringLiteral("Unknown");
}

void Worker::startBytesCounter()
{
    Q_ASSERT(_p->bytesCounterTimer == nullptr);

    _p->bytesCounterTimer = new QTimer(this);
    _p->bytesCounterTimer->setTimerType(Qt::TimerType::VeryCoarseTimer);
    _p->bytesCounterTimer->setSingleShot(false);
    _p->bytesCounterTimer->setInterval(1000);
    connect(_p->bytesCounterTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            Q_EMIT rxBytesCounterChanged(_p->rxBytesCounter);
            Q_EMIT txBytesCounterChanged(_p->txBytesCounter);
            Q_EMIT rxPacketsCounterChanged(_p->rxPacketsCounter);
            Q_EMIT txPacketsCounterChanged(_p->txPacketsCounter);
            Q_EMIT rxInvalidPacketsCounterChanged(_p->rxInvalidPacket);

            _p->rxBytesCounter = 0;
            _p->txBytesCounter = 0;
            _p->rxPacketsCounter = 0;
            _p->txPacketsCounter = 0;
            _p->rxInvalidPacket = 0;
        });
    _p->bytesCounterTimer->start();
}

void Worker::stopBytesCounter()
{
    Q_EMIT rxBytesCounterChanged(0);
    Q_EMIT txBytesCounterChanged(0);
    Q_EMIT rxPacketsCounterChanged(0);
    Q_EMIT txPacketsCounterChanged(0);

    if(_p->bytesCounterTimer)
        _p->bytesCounterTimer->deleteLater();
    _p->bytesCounterTimer = nullptr;
}

}
