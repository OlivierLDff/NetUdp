// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Worker.hpp>
#include <Net/Udp/Logger.hpp>
#include <Net/Udp/InterfacesProvider.hpp>

// Qt Core
#include <QUdpSocket>
#include <QTimer>
#include <QElapsedTimer>

// Qt Network
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QNetworkDatagram>

// Stl
#include <algorithm>

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

bool Worker::multicastLoopback() const { return _multicastLoopback; }

quint8 Worker::multicastTtl() const { return _multicastTtl; }

bool Worker::inputEnabled() const { return _inputEnabled; }

bool Worker::separateRxTxSocketsChanged() const { return _separateRxTxSockets; }

QUdpSocket* Worker::rxSocket() const
{
    if(_separateRxTxSockets)
        return _rxSocket;
    return _socket;
}

size_t Worker::cacheSize() const { return _cache.size(); }

bool Worker::resizeCache(size_t length) { return _cache.resize(length); }

void Worker::clearCache() { _cache.clear(); }

void Worker::releaseCache() { _cache.release(); }

std::shared_ptr<Datagram> Worker::makeDatagram(const size_t length) { return _cache.make(length); }

void Worker::onRestart()
{
    onStop();
    onStart();
}

void Worker::onStart()
{
    if(_socket)
    {
        LOG_DEV_ERR("Can't start udp socket worker because socket is already valid");
        return;
    }

    if(_inputEnabled)
    {
        if(_rxAddress.isEmpty())
            LOG_DEV_INFO(
                "Start Udp Socket Worker rx : {}:{}, tx port : {}", _rxAddress.toStdString(), _rxPort, _txPort);
        else
            LOG_DEV_INFO("Start Udp Socket Worker rx port : {}, tx port : {}", _rxPort, _txPort);
    }
    else
    {
        if(_rxAddress.isEmpty())
            LOG_DEV_INFO("Start Udp Socket Worker rx : {}:{}", _rxAddress.toStdString(), _rxPort);
        else
            LOG_DEV_INFO("Start Udp Socket Worker rx port : {}", _rxPort);
    }

    _isBounded = false;
    _watchdog = nullptr;

    _failedJoiningMulticastGroup.clear();
    _joinedMulticastGroups.clear();
    _allMulticastInterfaces.clear();

    // Create the socket (and a second one for rx if required)
    if(_socket)
        _socket->deleteLater();
    _socket = new QUdpSocket(this);
    const bool useTwoSockets = (_separateRxTxSockets || _txPort) && _inputEnabled;
    if(useTwoSockets)
    {
        LOG_DEV_INFO("Create separate rx socket");
        if(_rxSocket)
            _rxSocket->deleteLater();
        _rxSocket = new QUdpSocket(this);
    }

    connect(
        this, &Worker::queueStartWatchdog, this,
        [this]()
        {
            if(!_watchdog)
            {
                onStop();

                // ) Create a watchdog timer
                if(_watchdog)
                    _watchdog->deleteLater();
                _watchdog = new QTimer(this);
                _watchdog->setTimerType(Qt::TimerType::VeryCoarseTimer);

                // ) Connect timeout
                connect(
                    _watchdog, &QTimer::timeout, this, [this]() { onRestart(); }, Qt::ConnectionType::QueuedConnection);

                LOG_INFO("Start watchdog to restart socket in {} millis", watchdogTimeout());
                // Start the watchdog
                _watchdog->start(watchdogTimeout());
            }
            else
            {
                LOG_DEV_WARN("Watchdog already running, fail to start it");
            }
        },
        Qt::QueuedConnection);

    // Connect to socket signals
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(
        _socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Worker::onSocketError);
#else
    connect(_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this,
        &Worker::onSocketError);
#endif

    if(rxSocket())
    {
        connect(rxSocket(), &QUdpSocket::readyRead, this, &Worker::readPendingDatagrams);
        // _socket is always bounded because binded to QHostAddress(). Only rxSocket can go wrong
        connect(rxSocket(), &QUdpSocket::stateChanged, this, &Worker::onSocketStateChanged);
    }

    if(useTwoSockets)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        connect(_rxSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
            &Worker::onRxSocketError);
#else
        connect(_rxSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this,
            &Worker::onRxSocketError);
#endif
    }

    // Bind to socket
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
                   _socket->bind(
                       QHostAddress(), _txPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

        if(_inputEnabled)
            return _socket->bind(
                QHostAddress(_rxAddress), _rxPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
        return _socket->bind(
            QHostAddress(), _txPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    }();

    // Finish the start of the socket
    // Or start watchdog on failure
    if(bindSuccess)
    {
        LOG_INFO("Success bind to {}:{}", qPrintable(_socket->localAddress().toString()), _socket->localPort());

        if(!_multicastGroups.empty() && rxSocket() && _inputEnabled)
        {
            // Join multicast groups either on every interfaces or in '_incomingMulticastInterfaces' given by user
            if(_incomingMulticastInterfaces.empty())
            {
                tryJoinAllAvailableInterfaces();
            }
            else
            {
                for(const auto& interfaceName: _incomingMulticastInterfaces)
                {
                    for(const auto& group: _multicastGroups) joinAndTrackMulticastGroup(group, interfaceName);
                }
            }
        }

        setMulticastLoopbackToSocket();
        startBytesCounter();
    }
    else
    {
        LOG_ERR("Fail to bind to {} : {}", qPrintable(_rxAddress.isEmpty() ? "Any" : _rxAddress), _rxPort);
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

    stopListeningMulticastInterfaceWatcher();
    stopOutputMulticastInterfaceWatcher();
    stopBytesCounter();
    stopWatchdog();
    disconnect(this, nullptr, this, nullptr);

    if(_socket)
        _socket->deleteLater();
    _socket = nullptr;
    if(_rxSocket)
        _rxSocket->deleteLater();
    _rxSocket = nullptr;
    _multicastTtl = 0;

    // Delete every multicast outgoing socket
    for(const auto [iface, socket]: _multicastTxSockets) socket->deleteLater();
    _multicastTxSockets.clear();

    _failedJoiningMulticastGroup.clear();
    _joinedMulticastGroups.clear();
    _allMulticastInterfaces.clear();
    _failedToInstantiateMulticastTxSockets.clear();
    _multicastTxSocketsInstantiated = false;
}

void Worker::initialize(quint64 watchdog, QString rxAddress, quint16 rxPort, quint16 txPort, bool separateRxTxSocket,
    const std::set<QString>& multicastGroup, const std::set<QString>& multicastListeningInterfaces,
    const std::set<QString>& multicastOutgoingInterfaces, bool inputEnabled, bool multicastLoopback)
{
    _watchdogTimeout = watchdog;
    _rxAddress = rxAddress;
    _rxPort = rxPort;
    _txPort = txPort;
    _separateRxTxSockets = separateRxTxSocket;
    _multicastGroups = multicastGroup;
    _incomingMulticastInterfaces = multicastListeningInterfaces;
    _outgoingMulticastInterfaces = multicastOutgoingInterfaces;
    _inputEnabled = inputEnabled;
    _multicastLoopback = multicastLoopback;
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

void Worker::setInputEnabled(const bool enabled)
{
    LOG_DEV_INFO("Set Input Enabled : {}", enabled);
    if(enabled != _inputEnabled)
    {
        _inputEnabled = enabled;
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

    const auto [groupIt, groupInsertSuccess] = _multicastGroups.insert(address);

    // Group is already joined
    if(!groupInsertSuccess)
        return;

    // Input disable, we should listen to anything nor subscribe
    if(!_inputEnabled)
        return;

    // No rx socket mean that the socket isn't started. 'onStart' will take care of really joining the group.
    if(!rxSocket())
        return;

    if(_incomingMulticastInterfaces.empty())
    {
        if(_allMulticastInterfaces.empty())
        {
            tryJoinAllAvailableInterfaces();
        }
        else
        {
            // Join the group address an each interface an keep track of success or fail
            for(const auto& interfaceName: _allMulticastInterfaces) joinAndTrackMulticastGroup(address, interfaceName);
        }
    }
    else
    {
        // Join the group address an each interface an keep track of success or fail
        for(const auto& interfaceName: _incomingMulticastInterfaces) joinAndTrackMulticastGroup(address, interfaceName);
    }
}

void Worker::leaveMulticastGroup(const QString& address)
{
    LOG_INFO("Leave Multicast group {}", address.toStdString());

    const auto it = _multicastGroups.find(address);

    // Group isn't present
    if(it == _multicastGroups.end())
        return;

    _multicastGroups.erase(it);

    // No rx socket mean that the socket isn't started. It mean that no multicast group are also joined
    if(!rxSocket())
        return;

    std::vector<QString> interfaceNameJoinedToRemove;
    std::vector<QString> interfaceNameFailedToRemove;

    // Leave the multicast group on every interface that were joined successfully
    for(auto& [interfaceName, groups]: _joinedMulticastGroups)
    {
        // Really leave if the address was erased(ie found) in the set.
        if(groups.erase(address))
            socketLeaveMulticastGroup(address, interfaceName);

        if(groups.empty())
            interfaceNameJoinedToRemove.push_back(interfaceName);
    }

    // And also clean _failedJoiningMulticastGroup from the multicast group we are leaving
    for(auto& [interfaceName, groups]: _failedJoiningMulticastGroup)
    {
        groups.erase(address);

        if(groups.empty())
            interfaceNameFailedToRemove.push_back(interfaceName);
    }

    // Remove interfaces from that that have an empty key
    if(interfaceNameJoinedToRemove.size() == _joinedMulticastGroups.size())
    {
        _joinedMulticastGroups.clear();
    }
    else
    {
        for(const auto& name: interfaceNameJoinedToRemove) _joinedMulticastGroups.erase(name);
    }

    if(interfaceNameFailedToRemove.size() == _failedJoiningMulticastGroup.size())
    {
        _failedJoiningMulticastGroup.clear();
    }
    else
    {
        for(const auto& name: interfaceNameFailedToRemove) _failedJoiningMulticastGroup.erase(name);
    }

    if(_multicastGroups.empty())
        stopListeningMulticastInterfaceWatcher();
}

void Worker::joinMulticastInterface(const QString& interfaceName)
{
    if(_incomingMulticastInterfaces.empty())
        tryLeaveAllAvailableInterfaces();

    const auto [interfaceNameIt, interfaceNameInsertSuccess] = _incomingMulticastInterfaces.insert(interfaceName);
    const auto interface = InterfacesProvider::interfaceFromName(interfaceName);

    // Interface already exist. We don't need to do any actions
    if(!interfaceNameInsertSuccess)
        return;

    // Input disable, don't need to do anything
    if(!_inputEnabled)
        return;

    // No rx socket mean that the socket isn't started. 'onStart' will take care of really joining the group.
    if(!rxSocket())
        return;

    // Join each multicast group
    for(const auto& group: _multicastGroups) joinAndTrackMulticastGroup(group, interfaceName);
}

void Worker::leaveMulticastInterface(const QString& interfaceName)
{
    // If the interface wasn't present, then don't need to do anything
    if(!_incomingMulticastInterfaces.erase(interfaceName))
        return;

    // No rx socket mean that the socket isn't started. It mean that no multicast group are also joined
    if(!rxSocket())
        return;

    // Leave all group join on interface 'interfaceName'
    const auto it = _joinedMulticastGroups.find(interfaceName);
    if(it != _joinedMulticastGroups.end())
    {
        const auto& groupJoined = it->second;
        for(const auto& group: groupJoined) socketLeaveMulticastGroup(group, interfaceName);

        _joinedMulticastGroups.erase(it);
    }

    // Clear cache of failed to join multicast group
    _failedJoiningMulticastGroup.erase(interfaceName);

    // Try to listen on every interfaces if _incomingMulticastInterfaces is empty
    if(_incomingMulticastInterfaces.empty())
    {
        // Order matter because 'tryJoinAllAvailableInterfaces' might recreate the timer
        stopListeningMulticastInterfaceWatcher();
        tryJoinAllAvailableInterfaces();
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

void Worker::setMulticastOutgoingInterfaces(const QStringList& interfaces)
{
    // Nothing changed
    if(interfaces.empty() && _outgoingMulticastInterfaces.empty())
        return;

    // It will maybe be time to create new multicast socket
    if(_outgoingMulticastInterfaces.empty())
        destroyMulticastOutputSockets();

    // Copy interfaces to _outgoingMulticastInterfaces
    _outgoingMulticastInterfaces = MulticastInterfaceList(interfaces.begin(), interfaces.end());

    // Destroy what was already instantiated.
    if(_multicastTxSocketsInstantiated)
        destroyMulticastOutputSockets();
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

void Worker::tryJoinAllAvailableInterfaces()
{
    if(!_incomingMulticastInterfaces.empty())
        return;

    // Input disable, don't need to do anything
    if(!_inputEnabled)
        return;

    // No rx socket mean that the socket isn't started. 'onStart' will take care of really joining the group.
    if(!rxSocket())
        return;

    // Assert that model is really empty
    Q_ASSERT(_joinedMulticastGroups.empty());
    Q_ASSERT(_failedJoiningMulticastGroup.empty());

    // join every group on every interface
    // It is expected for interfaces to fail joining. Attempt to join will be made later.
    const auto& allInterfaces = InterfacesProvider::allInterfaces();
    for(const auto& interface: allInterfaces)
    {
        Q_ASSERT(interface);
        const auto [it, success] = _allMulticastInterfaces.insert(interface->name());
        if(!success)
            continue;

        for(const auto& group: _multicastGroups) { joinAndTrackMulticastGroup(group, interface->name()); }
    }
}

void Worker::tryLeaveAllAvailableInterfaces()
{
    if(!_incomingMulticastInterfaces.empty())
        return;

    // No rx socket mean that the socket isn't started. 'onStart' will take care of really joining the group.
    if(!rxSocket())
        return;

    for(const auto& [interfaceName, groups]: _joinedMulticastGroups)
    {
        for(const auto& group: groups) socketLeaveMulticastGroup(group, interfaceName);
    }

    _joinedMulticastGroups.clear();
    _failedJoiningMulticastGroup.clear();
    _allMulticastInterfaces.clear();
    stopListeningMulticastInterfaceWatcher();
}

bool Worker::joinAndTrackMulticastGroup(const QString& address, const QString& interfaceName)
{
    // Create a timer anyway, we have at least one interface to watch
    startListeningMulticastInterfaceWatcher();

    if(!socketJoinMulticastGroup(address, interfaceName))
    {
        _failedJoiningMulticastGroup[interfaceName].insert(address);
        return false;
    }

    const auto [insertedIt, groupInserted] = _joinedMulticastGroups[interfaceName].insert(address);

    // If assert here it mean something gone wrong in join/leave.
    Q_ASSERT(groupInserted);
    return true;
}

bool Worker::socketJoinMulticastGroup(const QString& address, const QString& interfaceName)
{
    const QHostAddress hostAddress(address);
    const auto networkInterface = QNetworkInterface::interfaceFromName(interfaceName);
    const auto networkInterfaceValid =
        networkInterface.isValid() && (networkInterface.flags() & QNetworkInterface::IsUp) &&
        (networkInterface.flags() & QNetworkInterface::IsRunning) &&
        ((networkInterface.flags() & QNetworkInterface::CanMulticast) ||
            (multicastLoopback() && (networkInterface.flags() & QNetworkInterface::IsLoopBack)));

    // should be verified by Socket
    Q_ASSERT(hostAddress.isMulticast());
    Q_ASSERT(rxSocket());
    Q_ASSERT(_inputEnabled);

    // Try to join the multicast group
    if(!networkInterfaceValid || !rxSocket()->joinMulticastGroup(hostAddress, networkInterface))
    {
        if(networkInterfaceValid)
            LOG_ERR("Fail to join multicast group {} an interface {}. {}", address.toStdString(),
                interfaceName.toStdString(), rxSocket()->errorString().toStdString());
        else
            LOG_WARN("Fail to join multicast group {} an interface {} because interface is not valid. IsUp:{}, "
                     "IsRunning:{}, CanMulticast:{}, IsLoopBack:{}",
                address.toStdString(), interfaceName.toStdString(),
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
        networkInterface.isValid() &&
        networkInterface.flags() & (QNetworkInterface::IsRunning | QNetworkInterface::IsUp);

    if(!networkInterfaceValid || !rxSocket()->leaveMulticastGroup(hostAddress, networkInterface))
    {
        if(networkInterfaceValid)
            LOG_ERR("Fail to leave multicast group {} an interface {}. {}", address.toStdString(),
                interfaceName.toStdString(), rxSocket()->errorString().toStdString());
        else
            LOG_ERR("Fail to leave multicast group {} an interface {} because interface is not valid",
                address.toStdString(), interfaceName.toStdString());
        return false;
    }

    LOG_INFO("Success Leave multicast group {} on interface {}", address.toStdString(), interfaceName.toStdString());
    Q_EMIT multicastGroupLeaved(address, interfaceName);

    return true;
}

void Worker::setMulticastLoopbackToSocket() const
{
    if(rxSocket() && _inputEnabled)
    {
        LOG_INFO("Set MulticastLoopbackOption to {}", int(_multicastLoopback));
        rxSocket()->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _multicastLoopback);
        if(rxSocket() != _socket)
            _socket->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _multicastLoopback);

        for(const auto [interfaceName, socket]: _multicastTxSockets)
        { socket->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _multicastLoopback); }
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
        // This should be set in case _multicastTxSockets is empty
        _multicastTtl = ttl;
        _socket->setSocketOption(QAbstractSocket::MulticastTtlOption, int(_multicastTtl ? _multicastTtl : 8));
        for(const auto [interfaceName, socket]: _multicastTxSockets)
        {
            socket->setSocketOption(
                QAbstractSocket::SocketOption::MulticastTtlOption, int(_multicastTtl ? _multicastTtl : 8));
        };
    }
}

void Worker::startListeningMulticastInterfaceWatcher()
{
    if(!_listeningMulticastInterfaceWatcher)
    {
        _listeningMulticastInterfaceWatcher = new QTimer(this);

        _listeningMulticastInterfaceWatcher->setInterval(2500);
        _listeningMulticastInterfaceWatcher->setTimerType(Qt::VeryCoarseTimer);
        _listeningMulticastInterfaceWatcher->setSingleShot(false);

        connect(
            _listeningMulticastInterfaceWatcher, &QTimer::timeout, this,
            [this]()
            {
                // Should be deleted if _multicastGroups is empty
                Q_ASSERT(!_multicastGroups.empty());

                // Fetch all interfaces
                const auto allInterfaces = InterfacesProvider::allInterfaces();

                // if auto joining every interface
                if(_incomingMulticastInterfaces.empty())
                {
                    // Look for new interface to join. Every interface found are added to the '_failedJoiningMulticastGroup'.
                    // They will be joined with the other failed to join one.
                    for(const auto& interface: allInterfaces)
                    {
                        const auto interfaceName = interface->name();
                        if(_allMulticastInterfaces.find(interfaceName) == _allMulticastInterfaces.end())
                        {
                            LOG_DEV_INFO(
                                "Find a new interface to join for multicast : {}", interfaceName.toStdString());
                            _allMulticastInterfaces.insert(interfaceName);
                            _failedJoiningMulticastGroup.insert({interfaceName, _multicastGroups});
                        }
                    }

                    std::vector<QString> interfaceNameToRemove;

                    // Look for interfaces that disappear
                    for(const auto& interfaceName: _allMulticastInterfaces)
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
                        const auto& interfaceJoinedIt = _joinedMulticastGroups.find(interfaceName);
                        if(interfaceJoinedIt != _joinedMulticastGroups.end())
                        {
                            for(const auto& group: interfaceJoinedIt->second)
                            { socketLeaveMulticastGroup(group, interfaceName); }

                            _joinedMulticastGroups.erase(interfaceJoinedIt);
                        }
                        _failedJoiningMulticastGroup.erase(interfaceName);
                        _allMulticastInterfaces.erase(interfaceName);
                    }
                }

                std::vector<QString> disconnectedInterfaceList;

                // Look for interface disconnection
                for(const auto& [interfaceName, groups]: _joinedMulticastGroups)
                {
                    // Only try to reconnect to interface that seems valid
                    if(const auto& interface = InterfacesProvider::interfaceFromName(interfaceName))
                    {
                        const bool interfaceValid =
                            interface->isValid() && interface->isRunning() && interface->isUp() &&
                            (interface->canMulticast() || (multicastLoopback() && interface->isLoopBack()));
                        if(!interfaceValid)
                        {
                            LOG_DEV_INFO("Interface {} isn't valid anymore, It will be removed from joined interfaces. "
                                         "The worker will try to re join later the interface",
                                interfaceName.toStdString());
                            const auto& currentGroups = _failedJoiningMulticastGroup[interfaceName];

                            // Move the list of joined multicast group to the failed one.
                            if(currentGroups.empty())
                            {
                                _failedJoiningMulticastGroup[interfaceName] = groups;
                            }
                            else
                            {
                                MulticastGroupList newList;
                                std::merge(currentGroups.begin(), currentGroups.end(), groups.begin(), groups.end(),
                                    std::inserter(newList, newList.begin()));
                                _failedJoiningMulticastGroup[interfaceName] = newList;
                            }

                            // And request a delete later (since we are iterating in this map)
                            disconnectedInterfaceList.push_back(interfaceName);
                        }
                    }
                }

                for(const auto& disconnectedInterfaceName: disconnectedInterfaceList)
                    _joinedMulticastGroups.erase(disconnectedInterfaceName);

                // Try to rejoin every groups
                for(auto& [interfaceName, groups]: _failedJoiningMulticastGroup)
                {
                    // Only try to reconnect to interface that seems valid
                    if(const auto& interface = InterfacesProvider::interfaceFromName(interfaceName))
                    {
                        const bool interfaceValid = interface->isValid() && interface->isRunning() &&
                                                    interface->isUp() &&
                                                    (multicastLoopback() && interface->isLoopBack());
                        if(!interfaceValid)
                            continue;
                    }

                    std::vector<QString> successFullyJoinedGroup;

                    for(const auto& group: groups)
                    {
                        if(joinAndTrackMulticastGroup(group, interfaceName))
                            successFullyJoinedGroup.push_back(group);
                    }

                    for(const auto& group: successFullyJoinedGroup) { groups.erase(group); }
                }

                // Remove interfaces with empty group
                for(auto it = _failedJoiningMulticastGroup.begin(); it != _failedJoiningMulticastGroup.end();)
                {
                    if(it->second.empty())
                        it = _failedJoiningMulticastGroup.erase(it);
                    else
                        ++it;
                }

                // Stop timer if nothing left to watch
                if(_joinedMulticastGroups.empty() || _failedJoiningMulticastGroup.empty())
                    stopListeningMulticastInterfaceWatcher();
            },
            Qt::QueuedConnection);

        _listeningMulticastInterfaceWatcher->start();
    }
}

void Worker::stopListeningMulticastInterfaceWatcher()
{
    if(_listeningMulticastInterfaceWatcher)
    {
        _listeningMulticastInterfaceWatcher->deleteLater();
        _listeningMulticastInterfaceWatcher = nullptr;
    }
}

void Worker::startOutputMulticastInterfaceWatcher()
{
    if(!_outputMulticastInterfaceWatcher)
    {
        _outputMulticastInterfaceWatcher = new QTimer(this);
        _outputMulticastInterfaceWatcher->setInterval(2500);
        _outputMulticastInterfaceWatcher->setSingleShot(false);
        _outputMulticastInterfaceWatcher->setTimerType(Qt::VeryCoarseTimer);

        connect(_outputMulticastInterfaceWatcher, &QTimer::timeout, this,
            [this]()
            {
                // If too much time without datagram send, then we destroy every sockets
                Q_CHECK_PTR(_txMulticastPacketElapsedTime);
                // todo : set to 10000
                if(_txMulticastPacketElapsedTime->elapsed() > 100000)
                {
                    destroyMulticastOutputSockets();
                    return;
                }

                const auto allInterfaces = InterfacesProvider::allInterfaces();

                // When instantiating sockets for all interfaces, check if new interfaces appeared
                if(_outgoingMulticastInterfaces.empty())
                {
                    // Try to find if new interfaces were created and try to join them
                    for(const auto& interface: allInterfaces)
                    {
                        Q_CHECK_PTR(interface);
                        const auto interfaceName = interface->name();
                        const auto multicastTxSocketFound =
                            _multicastTxSockets.find(interfaceName) != _multicastTxSockets.end();
                        const auto multicastFailedToCreateFound =
                            _failedToInstantiateMulticastTxSockets.find(interfaceName) !=
                            _failedToInstantiateMulticastTxSockets.end();

                        // New interface detected, It's added to the list of _failedToInstantiateMulticastTxSockets
                        // Then it will be instantiated by the step of the algorithm.
                        if(!multicastFailedToCreateFound && !multicastTxSocketFound)
                            _failedToInstantiateMulticastTxSockets.insert(interfaceName);
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
                for(auto it = _failedToInstantiateMulticastTxSockets.begin();
                    it != _failedToInstantiateMulticastTxSockets.end();)
                {
                    const auto interfaceName = *it;
                    if(isInterfacePresent(interfaceName))
                    {
                        ++it;
                    }
                    else
                    {
                        LOG_DEV_INFO(
                            "Detect interface {} disappear, stop trying to instantiate a udp multicast socket for it",
                            interfaceName.toStdString());
                        it = _failedToInstantiateMulticastTxSockets.erase(it);
                    }
                }
                for(auto it = _multicastTxSockets.begin(); it != _multicastTxSockets.end();)
                {
                    const auto [interfaceName, socket] = *it;
                    if(isInterfacePresent(interfaceName))
                    {
                        ++it;
                    }
                    else
                    {
                        LOG_DEV_INFO("Detect interface {} disappear, delete the associated multicast socket",
                            interfaceName.toStdString());
                        socket->deleteLater();
                        it = _multicastTxSockets.erase(it);
                    }
                }

                const auto failedToInstantiateMulticastTxSocketsCopy = _failedToInstantiateMulticastTxSockets;
                _failedToInstantiateMulticastTxSockets.clear();

                // Then check all the socket that failed to be instantiated and try again.
                for(const auto& interfaceName: failedToInstantiateMulticastTxSocketsCopy)
                {
                    const auto interface = InterfacesProvider::interfaceFromName(interfaceName);
                    Q_CHECK_PTR(interface);
                    createMulticastSocketForInterface(*interface);
                }
            });

        _outputMulticastInterfaceWatcher->start();
    }
}

void Worker::stopOutputMulticastInterfaceWatcher()
{
    if(_outputMulticastInterfaceWatcher)
    {
        _outputMulticastInterfaceWatcher->deleteLater();
        _outputMulticastInterfaceWatcher = nullptr;
    }
}

void Worker::createMulticastSocketForInterface(const IInterface& interface)
{
    const auto successCreateSocket = [&]() -> bool
    {
        const auto interfaceName = interface.name();
        const bool isInterfaceValid = interface.isValid() && interface.isUp() && interface.isRunning() &&
                                      (interface.canMulticast() || (multicastLoopback() && interface.isLoopBack()));

        if(!isInterfaceValid)
        {
            LOG_DEV_WARN(
                "Can't create multicast socket for interface {} because it's not valid: isValid: {}, isUp: {}, "
                "isRunning: {}, canMulticast: {}, isLoopback: {}, multicastLoopback: {}",
                interface.name().toStdString(), interface.isValid(), interface.isUp(), interface.isRunning(),
                interface.canMulticast(), interface.isLoopBack(), multicastLoopback());
            return false;
        }

        if(_multicastTxSockets.find(interfaceName) != _multicastTxSockets.end())
        {
            LOG_DEV_WARN("Multicast tx socket is already instantiated for interface {}. This might hide a bug in "
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
        socket->setSocketOption(QAbstractSocket::SocketOption::MulticastLoopbackOption, _multicastLoopback);

        //const auto returnedInterface = socket->multicastInterface();
        //LOG_DEV_INFO("returnedInterface.isValid : {}", returnedInterface.isValid());
        //LOG_DEV_INFO("returnedInterface.name : {}", returnedInterface.name().toStdString());

        const auto onError = [interfaceName, socket, this](QAbstractSocket::SocketError error)
        {
            LOG_DEV_WARN("Multicast tx error {}", socket->errorString().toStdString());
            socket->deleteLater();
            _multicastTxSockets.erase(interfaceName);
            _failedToInstantiateMulticastTxSockets.insert(interfaceName);
        };

        // Connect to socket signals
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, onError,
            Qt::QueuedConnection);
#else
        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, onError,
            Qt::QueuedConnection);
#endif

        const auto [it, success] = _multicastTxSockets.insert({interfaceName, socket});

        // This have been checked before creating the socket if(_multicastTxSockets.find(interfaceName) != _multicastTxSockets.end())
        Q_ASSERT(success);

        LOG_DEV_INFO("Create multicast tx socket for interface {}", interfaceName.toStdString());

        return true;
    }();

    if(!successCreateSocket)
        _failedToInstantiateMulticastTxSockets.insert(interface.name());
};

void Worker::createMulticastOutputSockets()
{
    if(_multicastTxSocketsInstantiated)
    {
        LOG_DEV_WARN("Can't create Multicast output sockets because they are already created. You should call "
                     "'destroyMulticastOutputSockets' before.");
        return;
    }

    // If _multicastTxSocketsInstantiated is false, _multicastTxSockets HAVE TO be empty
    Q_ASSERT(_multicastTxSockets.empty());

    // Create sockets only for the interfaces specified by user or for every interfaces if nothing is specified
    if(_outgoingMulticastInterfaces.empty())
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
        for(const auto& interfaceName: _outgoingMulticastInterfaces)
        {
            const auto interface = InterfacesProvider::interfaceFromName(interfaceName);
            Q_CHECK_PTR(interface);
            createMulticastSocketForInterface(*interface);
        }
    }

    _txMulticastPacketElapsedTime = std::make_unique<QElapsedTimer>();
    startOutputMulticastInterfaceWatcher();
    _multicastTxSocketsInstantiated = true;
}

void Worker::destroyMulticastOutputSockets()
{
    LOG_DEV_INFO("Destroy all multicast tx sockets");
    for(const auto [interfaceName, socket]: _multicastTxSockets)
    {
        Q_CHECK_PTR(socket);
        socket->deleteLater();
    }
    _multicastTxSockets.clear();
    _failedJoiningMulticastGroup.clear();
    stopOutputMulticastInterfaceWatcher();
    _multicastTxSocketsInstantiated = false;
    _txMulticastPacketElapsedTime = nullptr;
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
            QNetworkDatagram d(QByteArray(reinterpret_cast<const char*>(datagram->buffer()), int(datagram->length())),
                host, datagram->destinationPort);

            d.setHopLimit(datagram->ttl ? datagram->ttl : -1);
            return _socket->writeDatagram(d);
        }

        // If multi
        if(isMulticast)
        {
            if(!_multicastTxSocketsInstantiated)
                createMulticastOutputSockets();

            if(!_multicastTxSockets.empty())
            {
                bool byteWrittenInitialized = false;
                qint64 bytes = 0;
                for(const auto [interfaceName, socket]: _multicastTxSockets)
                {
                    socket->setSocketOption(
                        QAbstractSocket::MulticastTtlOption, int(_multicastTtl ? _multicastTtl : 8));
                    const auto currentBytesWritten =
                        socket->writeDatagram(reinterpret_cast<const char*>(datagram->buffer()), datagram->length(),
                            host, datagram->destinationPort);

                    if(!byteWrittenInitialized)
                    {
                        byteWrittenInitialized = true;
                        bytes = currentBytesWritten;
                    }
                }

                // _timeSinceNoTxMulticastDatagram can't be null when _multicastTxSockets are instantiated
                Q_CHECK_PTR(_txMulticastPacketElapsedTime);
                _txMulticastPacketElapsedTime->start();
                return bytes;
            }
        }

        return _socket->writeDatagram(
            reinterpret_cast<const char*>(datagram->buffer()), datagram->length(), host, datagram->destinationPort);
    }();

    if(bytesWritten <= 0 || bytesWritten != datagram->length())
    {
        startWatchdog();

        if(bytesWritten <= 0)
            LOG_ERR("Fail to send datagram, 0 bytes written. Restart Socket. {}", _socket->errorString().toStdString());
        else
            LOG_ERR("Fail to send datagram, {}/{} bytes written. Restart Socket. {}",
                static_cast<long long>(bytesWritten), static_cast<long long>(datagram->length()),
                _socket->errorString().toStdString());
        return;
    }

    _txBytesCounter += bytesWritten;
    ++_txPacketsCounter;
}

bool Worker::isPacketValid(const uint8_t* buffer, const size_t length) const { return buffer && length; }

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

        if(!isPacketValid(reinterpret_cast<const uint8_t*>(datagram.data().constData()), datagram.data().size()))
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
        memcpy(sharedDatagram.get()->buffer(), reinterpret_cast<const uint8_t*>(datagram.data().constData()),
            datagram.data().size());
        sharedDatagram->destinationAddress = datagram.destinationAddress().toString();
        if(datagram.destinationPort() >= 0)
            sharedDatagram->destinationPort = datagram.destinationPort();
        sharedDatagram->senderAddress = datagram.senderAddress().toString();
        if(datagram.senderPort() >= 0)
            sharedDatagram->senderPort = datagram.senderPort();
        if(datagram.hopLimit() >= 0)
            sharedDatagram->ttl = datagram.hopLimit();

        _rxBytesCounter += datagram.data().size();
        ++_rxPacketsCounter;

        onDatagramReceived(sharedDatagram);
    }
}

void Worker::onDatagramReceived(const SharedDatagram& datagram) { Q_EMIT datagramReceived(datagram); }

void Worker::onSocketError(QAbstractSocket::SocketError error) { onSocketErrorCommon(error, _socket); }

void Worker::onRxSocketError(QAbstractSocket::SocketError error) { onSocketErrorCommon(error, _rxSocket); }

void Worker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    LOG_INFO("Socket State Changed to {} ({})", socketStateToString(socketState).toStdString(), socketState);

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
    Q_ASSERT(_bytesCounterTimer == nullptr);

    _bytesCounterTimer = new QTimer(this);
    _bytesCounterTimer->setTimerType(Qt::TimerType::VeryCoarseTimer);
    _bytesCounterTimer->setSingleShot(false);
    _bytesCounterTimer->setInterval(1000);
    connect(_bytesCounterTimer, &QTimer::timeout, this,
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

    if(_bytesCounterTimer)
        _bytesCounterTimer->deleteLater();
    _bytesCounterTimer = nullptr;
}
