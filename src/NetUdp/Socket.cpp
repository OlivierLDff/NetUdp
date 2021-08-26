// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <NetUdp/Socket.hpp>
#include <NetUdp/Worker.hpp>
#include <NetUdp/RecycledDatagram.hpp>
#include <QtCore/QThread>
#include <QtCore/QLoggingCategory>
#include <QtCore/QDebug>
#include <QtNetwork/QHostAddress>
#include <Recycler/Circular.hpp>
#include <limits>

Q_LOGGING_CATEGORY(netudp_socket_log, "netudp.socket");

namespace netudp {

struct SocketPrivate
{
    Worker* worker = nullptr;
    QThread* workerThread = nullptr;

    // Recycle datagram to reduce dynamic allocation
    recycler::Circular<RecycledDatagram> cache;

    // Multicast group to which the socket subscribe
    std::set<QString> multicastListeningGroups;

    // Network Interfaces on which multicast groups are listened.
    std::set<QString> multicastListeningInterfaces;

    // Network INterfaces on which multicast datagram are send
    std::set<QString> multicastOutgoingInterfaces;
};

ISocket::ISocket(QObject* parent)
    : QObject(parent)
{
}

Socket::Socket(QObject* parent)
    : ISocket(parent)
    , _p(std::make_unique<SocketPrivate>())
{
}

Socket::~Socket()
{
    killWorker();
}

void Socket::killWorker()
{
    if(!_p->worker)
        return;

    qCDebug(netudp_socket_log) << "Stop Worker " << static_cast<void*>(_p->worker);
    Q_EMIT stopWorker();

    qCDebug(netudp_socket_log) << "Disconnect Worker " << static_cast<void*>(_p->worker);
    Q_ASSERT(_p->worker);
    disconnect(_p->worker, nullptr, this, nullptr);
    disconnect(this, nullptr, _p->worker, nullptr);

    if(_p->workerThread)
    {
        // Ask to delete later in the event loop
        qCDebug(netudp_socket_log) << "Kill worker thread ...";
        _p->workerThread->quit();
        _p->workerThread->wait();
        _p->workerThread->deleteLater();
        _p->workerThread = nullptr;
        // Worker will be deleted with the finished signal from QThread
        _p->worker = nullptr;
        qCDebug(netudp_socket_log) << "... Done";
    }
    else if(_p->worker)
    {
        qCDebug(netudp_socket_log) << "Delete worker later" << static_cast<void*>(_p->worker);
        _p->worker->deleteLater();
        _p->worker = nullptr;
    }
}

bool Socket::setUseWorkerThread(const bool& enabled)
{
    if(ISocket::setUseWorkerThread(enabled))
    {
        if(isRunning())
        {
            qCDebug(netudp_socket_log) << "Use worker thread change to " << enabled << ", restart server to reflect change on worker.";
        }
        else
        {
            qCDebug(netudp_socket_log) << "Use worker thread change to " << enabled;
        }

        if(isRunning())
            restart();
        return true;
    }
    return false;
}

QStringList Socket::multicastGroups() const
{
    return QList<QString>(_p->multicastListeningGroups.begin(), _p->multicastListeningGroups.end());
}

bool Socket::setMulticastGroups(const QStringList& value)
{
    leaveAllMulticastGroups();
    for(const auto& group: value)
    {
        joinMulticastGroup(group);
    }
    return true;
}

QStringList Socket::multicastListeningInterfaces() const
{
    return QList<QString>(_p->multicastListeningInterfaces.begin(), _p->multicastListeningInterfaces.end());
}

bool Socket::setMulticastListeningInterfaces(const QStringList& values)
{
    bool changedHappened = false;

    // Join the new interfaces
    for(const auto& interfaceName: values)
    {
        if(_p->multicastListeningInterfaces.find(interfaceName) == _p->multicastListeningInterfaces.end())
        {
            changedHappened = true;
            joinMulticastInterface(interfaceName);
        }
    }

    // Leave interfaces not present anymore
    std::vector<QString> interfacesToLeave;
    for(const auto& interfaceName: _p->multicastListeningInterfaces)
    {
        if(!values.contains(interfaceName))
        {
            changedHappened = true;
            interfacesToLeave.push_back(interfaceName);
        }
    }
    for(const auto& interfaceName: interfacesToLeave)
    {
        leaveMulticastInterface(interfaceName);
    }

    return changedHappened;
}

bool Socket::start()
{
    if(isRunning())
    {
        qCWarning(netudp_socket_log) << "Can't start socket that is already running. Please call 'stop' before.";
        return false;
    }

    setRunning(true);

    Q_ASSERT(_p->worker == nullptr);
    Q_ASSERT(_p->workerThread == nullptr);

    _p->worker = createWorker();

    if(useWorkerThread())
    {
        _p->workerThread = new QThread(this);

        if(objectName().size())
            _p->workerThread->setObjectName(objectName() + " Worker");
        else
            _p->workerThread->setObjectName("UdpSocket Thread");

        _p->worker->moveToThread(_p->workerThread);
        connect(_p->workerThread, &QThread::finished, _p->worker, &QObject::deleteLater);
    }
    else
    {
        _p->worker->setParent(this);
    }

    _p->worker->setObjectName("Udp Worker");

    _p->worker->initialize(watchdogPeriod(),
        rxAddress(),
        rxPort(),
        txPort(),
        separateRxTxSockets(),
        _p->multicastListeningGroups,
        _p->multicastListeningInterfaces,
        _p->multicastOutgoingInterfaces,
        inputEnabled(),
        multicastLoopback());

    connect(this, &Socket::startWorker, _p->worker, &Worker::onStart);
    connect(this, &Socket::stopWorker, _p->worker, &Worker::onStop);
    connect(this, &Socket::restartWorker, _p->worker, &Worker::onRestart);

    connect(this, &Socket::joinMulticastGroupWorker, _p->worker, &Worker::joinMulticastGroup);
    connect(this, &Socket::leaveMulticastGroupWorker, _p->worker, &Worker::leaveMulticastGroup);

    connect(this, &Socket::joinMulticastInterfaceWorker, _p->worker, &Worker::joinMulticastInterface);
    connect(this, &Socket::leaveMulticastInterfaceWorker, _p->worker, &Worker::leaveMulticastInterface);

    connect(this, &Socket::rxAddressChanged, _p->worker, &Worker::setAddress);
    connect(this, &Socket::rxPortChanged, _p->worker, &Worker::setRxPort);
    connect(this, &Socket::txPortChanged, _p->worker, &Worker::setTxPort);
    connect(this, &Socket::separateRxTxSocketsChanged, _p->worker, &Worker::setSeparateRxTxSockets);
    connect(this, &Socket::multicastLoopbackChanged, _p->worker, &Worker::setMulticastLoopback);
    connect(this, &Socket::multicastOutgoingInterfacesChanged, _p->worker, &Worker::setMulticastOutgoingInterfaces);
    connect(this, &Socket::inputEnabledChanged, _p->worker, &Worker::setInputEnabled);
    connect(this, &Socket::watchdogPeriodChanged, _p->worker, &Worker::setWatchdogTimeout);

    connect(this, &Socket::sendDatagramToWorker, _p->worker, &Worker::onSendDatagram, Qt::QueuedConnection);
    connect(_p->worker, &Worker::datagramReceived, this, &Socket::onDatagramReceived, Qt::QueuedConnection);

    connect(_p->worker, &Worker::isBoundedChanged, this, &Socket::setBounded);
    connect(_p->worker, &Worker::socketError, this, &Socket::socketError);

    connect(_p->worker, &Worker::rxBytesCounterChanged, this, &Socket::onWorkerRxPerSecondsChanged);
    connect(_p->worker, &Worker::txBytesCounterChanged, this, &Socket::onWorkerTxPerSecondsChanged);
    connect(_p->worker, &Worker::rxPacketsCounterChanged, this, &Socket::onWorkerPacketsRxPerSecondsChanged);
    connect(_p->worker, &Worker::txPacketsCounterChanged, this, &Socket::onWorkerPacketsTxPerSecondsChanged);

    connect(_p->worker, &Worker::multicastGroupJoined, this, &Socket::multicastGroupJoined);
    connect(_p->worker, &Worker::multicastGroupLeaved, this, &Socket::multicastGroupLeaved);

    if(_p->workerThread)
    {
        qCDebug(netudp_socket_log) << "Start worker thread " << _p->workerThread;
        _p->workerThread->start();
    }

    qCDebug(netudp_socket_log) << "Start worker thread " << _p->worker;
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
    if(!isRunning())
        return false;

    setBounded(false);
    setRunning(false);

    resetRxBytesPerSeconds();
    resetTxBytesPerSeconds();
    resetRxPacketsPerSeconds();
    resetTxPacketsPerSeconds();

    _p->cache.clear();

    killWorker();

    return true;
}

bool Socket::joinMulticastGroup(const QString& groupAddress)
{
    // ) Check that the address isn't already registered
    if(_p->multicastListeningGroups.find(groupAddress) != _p->multicastListeningGroups.end())
        return false;

    // ) Check that this is a real multicast address
    if(!QHostAddress(groupAddress).isMulticast())
        return false;

    // ) Insert in the set and emit signal to say the multicast list changed
    _p->multicastListeningGroups.insert(groupAddress);
    Q_EMIT multicastGroupsChanged(multicastGroups());

    Q_EMIT joinMulticastGroupWorker(groupAddress);
    return true;
}

bool Socket::leaveMulticastGroup(const QString& groupAddress)
{
    const auto it = _p->multicastListeningGroups.find(groupAddress);

    // ) Is the multicast group present
    if(it == _p->multicastListeningGroups.end())
        return false;

    // ) Remove the multicast address from the list, then emit a signal to say the list changed
    _p->multicastListeningGroups.erase(it);
    Q_EMIT multicastGroupsChanged(multicastGroups());

    Q_EMIT leaveMulticastGroupWorker(groupAddress);
    return true;
}

bool Socket::leaveAllMulticastGroups()
{
    bool allSuccess = true;
    while(!_p->multicastListeningGroups.empty())
    {
        // Copy is required here because leaveMulticastGroup will erase the iterator
        const auto group = *_p->multicastListeningGroups.begin();
        if(!leaveMulticastGroup(group))
            allSuccess = false;
    }
    return allSuccess;
}

bool Socket::isMulticastGroupPresent(const QString& groupAddress)
{
    return _p->multicastListeningGroups.find(groupAddress) != _p->multicastListeningGroups.end();
}

bool Socket::joinMulticastInterface(const QString& name)
{
    // ) Check that the address isn't already registered
    if(_p->multicastListeningInterfaces.find(name) != _p->multicastListeningInterfaces.end())
        return false;

    // ) Insert in the set and emit signal to say the multicast list changed
    _p->multicastListeningInterfaces.insert(name);
    Q_EMIT multicastListeningInterfacesChanged(multicastListeningInterfaces());

    Q_EMIT joinMulticastInterfaceWorker(name);
    return true;
}

bool Socket::leaveMulticastInterface(const QString& name)
{
    const auto it = _p->multicastListeningInterfaces.find(name);

    // ) Is the multicast interface present
    if(it == _p->multicastListeningInterfaces.end())
        return false;

    // ) Remove the multicast address from the list, then emit a signal to say the list changed
    _p->multicastListeningInterfaces.erase(it);
    Q_EMIT multicastListeningInterfacesChanged(multicastListeningInterfaces());

    Q_EMIT leaveMulticastInterfaceWorker(name);
    return true;
}

bool Socket::leaveAllMulticastInterfaces()
{
    bool allSuccess = true;
    while(!_p->multicastListeningInterfaces.empty())
    {
        // Copy is required here because leaveMulticastInterface will erase the iterator
        const auto interface = *_p->multicastListeningInterfaces.begin();
        if(!leaveMulticastInterface(interface))
            allSuccess = false;
    }
    return allSuccess;
}

bool Socket::isMulticastInterfacePresent(const QString& name)
{
    return _p->multicastListeningInterfaces.find(name) != _p->multicastListeningInterfaces.end();
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

void Socket::clearRxInvalidCounter()
{
    resetRxInvalidPacketTotal();
}

void Socket::clearCounters()
{
    clearRxCounter();
    clearTxCounter();
}

Worker* Socket::createWorker()
{
    return new Worker;
}

std::shared_ptr<Datagram> Socket::makeDatagram(const size_t length)
{
    return _p->cache.make(length);
}

bool Socket::sendDatagram(const uint8_t* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl)
{
    if(!isSendDatagramAllowed())
        return false;

    if(!buffer)
    {
        qCWarning(netudp_socket_log) << "Fail to send null datagram";
        return false;
    }

    if(length <= 0)
    {
        qCWarning(netudp_socket_log) << "Fail to send datagram because the length is <= 0";
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

bool Socket::sendDatagram(const char* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl)
{
    return sendDatagram(reinterpret_cast<const uint8_t*>(buffer), length, address, port, ttl);
}

bool Socket::sendDatagram(std::shared_ptr<Datagram> datagram, const QString& address, const uint16_t port, const uint8_t ttl)
{
    if(!datagram)
    {
        qCWarning(netudp_socket_log) << "Fail to send null datagram";
        return false;
    }

    datagram->destinationAddress = address;
    datagram->destinationPort = port;
    datagram->ttl = ttl;
    return sendDatagram(std::move(datagram));
}

bool Socket::sendDatagram(std::shared_ptr<Datagram> datagram)
{
    if(!isSendDatagramAllowed())
        return false;

    if(!datagram || !datagram->buffer())
    {
        qCWarning(netudp_socket_log) << "Fail to send null datagram";
        return false;
    }

    if(datagram->length() <= 0)
    {
        qCWarning(netudp_socket_log) << "Fail to send datagram because the length is <= 0";
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
        qCWarning(netudp_socket_log) << "Can't send datagram because no 'address' specified";
        return false;
    }

    if(const auto property = datagram.property(portKey); property.isNumber())
    {
        const auto rawPort = property.toUInt();
        const auto maxPort = std::numeric_limits<quint16>::max();
        if(rawPort > maxPort)
        {
            qCWarning(netudp_socket_log) << "Can't send datagram because 'port' is out of bound " << rawPort << ">" << int(maxPort);
            return false;
        }
        port = quint16(property.toUInt());
    }
    else
    {
        qCWarning(netudp_socket_log) << "Can't send datagram because no 'port' specified";
        return false;
    }

    if(const auto property = datagram.property(ttlKey); property.isNumber())
    {
        const auto rawTtl = property.toUInt();
        const auto maxTtl = std::numeric_limits<quint8>::max();
        if(rawTtl > maxTtl)
        {
            qCWarning(netudp_socket_log) << "'ttl' will be default to os because the value is out of bound " << rawTtl << ">" << maxTtl;
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
            for(int i = 0; i < length; ++i)
            {
                sharedDatagram->buffer()[i] = property.property(i).toUInt();
            }
        }
        else
        {
            qCWarning(netudp_socket_log) << "Can't send datagram because no 'data' is unknown type";
            return false;
        }
    }
    else
    {
        qCWarning(netudp_socket_log) << "Can't send datagram because no 'data' specified";
        return false;
    }
    Q_ASSERT(sharedDatagram);

    sharedDatagram->destinationAddress = address;
    sharedDatagram->destinationPort = port;
    sharedDatagram->ttl = ttl;

    return sendDatagram(sharedDatagram);
}

bool Socket::isSendDatagramAllowed() const
{
    if(!isRunning() && !isBounded())
    {
        if(!isRunning())
        {
            qCWarning(netudp_socket_log) << "Fail to send datagram because the Udp socket isn't running";
        }
        else if(!isBounded())
        {
            qCWarning(netudp_socket_log) << "Fail to send datagram because the Udp Server isn't bounded to any "
                                            "interfaces.";
        }
        return false;
    }

    return true;
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
        const QJSValue jsData(QString::fromLatin1(reinterpret_cast<const char*>(datagram->buffer()), int(datagram->length())));
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

#include "moc_Socket.cpp"
