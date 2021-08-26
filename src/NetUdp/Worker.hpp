// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __NETUDP_SERVER_WORKER_HPP__
#define __NETUDP_SERVER_WORKER_HPP__

#include <NetUdp/RecycledDatagram.hpp>
#include <NetUdp/Export.hpp>
#include <NetUdp/InterfacesProvider.hpp>
#include <Recycler/Circular.hpp>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QElapsedTimer>
#include <QtNetwork/QAbstractSocket>

QT_FORWARD_DECLARE_CLASS(QUdpSocket);
QT_FORWARD_DECLARE_CLASS(QTimer);
QT_FORWARD_DECLARE_CLASS(QElapsedTimer);

#include <set>
#include <map>
#include <memory>

namespace netudp {

class NETUDP_API_ Worker : public QObject
{
    Q_OBJECT

    // ──────── CONSTRUCTOR ────────
public:
    Worker(QObject* parent = nullptr);
    ~Worker();

    using MulticastGroupList = std::set<QString>;
    using MulticastInterfaceList = std::set<QString>;
    using InterfaceToMulticastSocket = std::map<QString, QUdpSocket*>;

    // ──────── ATTRIBUTE ────────
private:
    QUdpSocket* _socket = nullptr;
    QUdpSocket* _rxSocket = nullptr;
    QTimer* _watchdog = nullptr;
    recycler::Circular<RecycledDatagram> _cache;
    bool _isBounded = false;
    quint64 _watchdogTimeout = 5000;
    QString _rxAddress;
    quint16 _rxPort = 0;
    quint16 _txPort = 0;

    // ─── Multicast - Input ───

    // List of all multicast group the socket is listening to
    MulticastGroupList _multicastGroups;

    // Incoming interfaces for multicast packets that are going to be listened.
    // If empty, then every interfaces available on your system that support multicast are going to be listened.
    // This set doesn't reflect the interfaces that are really joined. If you want to know which interfaces are really joined
    // or which one failed to joined, then check `_failedToJoinIfaces` & '_joinedIfaces'
    MulticastInterfaceList _incomingMulticastInterfaces;
    MulticastInterfaceList _allMulticastInterfaces;

    MulticastInterfaceList& currentMulticastInterfaces()
    {
        return _incomingMulticastInterfaces.empty() ? _allMulticastInterfaces : _incomingMulticastInterfaces;
    }

    // Keep track of all multicast interfaces, to know which one are available
    //std::set<QString> _allMulticastInterfaces;

    // Keep track of group that were joined on each interfaces
    std::map<QString, MulticastGroupList> _joinedMulticastGroups;

    // Keep track of the interfaces that couldn't be joined, to retry periodically when the interface will be up again
    // See 'startListeningMulticastInterfaceWatcher'
    std::map<QString, MulticastGroupList> _failedJoiningMulticastGroup;

    // ─── Multicast - Output ───

    // User requested outgoing multicast interfaces
    MulticastInterfaceList _outgoingMulticastInterfaces;

    // Keep track of interfaces for which sockets couldn't be created and try later.
    // See 'startOutputMulticastInterfaceWatcher'
    MulticastInterfaceList _failedToInstantiateMulticastTxSockets;

    // Create a socket for each interface on which we want to multicast
    // This is way more efficient than changing IP_MULTICAST_IF at each packet send.
    // _socket won't be used for multicast packet sending
    // Port used for each socket will be the one specified by txPort
    InterfaceToMulticastSocket _multicastTxSockets;

    // Indicate if _multicastSockets are created or not.
    // !_multicastSockets.empty() can't be used since it's possible that they is no interface at all.
    // In that case the regular _socket should be used to send datagram.
    // Creation and destruction is controlled by startMulticastOutputSockets/stopMulticastOutputSockets.
    // Each time _multicastSocketsInstantiated to true, a QElapsedTimer is instantiated.
    // If no multicast datagram are send for 30s, then all sockets are destroyed.²
    bool _multicastTxSocketsInstantiated = false;

    bool _multicastLoopback = false;
    quint8 _multicastTtl = 0;
    bool _inputEnabled = false;
    bool _separateRxTxSockets = false;

public:
    bool isBounded() const;
    quint64 watchdogTimeout() const;
    QString rxAddress() const;
    quint16 rxPort() const;
    quint16 txPort() const;
    bool multicastLoopback() const;
    quint8 multicastTtl() const;
    bool inputEnabled() const;
    bool separateRxTxSocketsChanged() const;

    QUdpSocket* rxSocket() const;

    size_t cacheSize() const;
    bool resizeCache(size_t length);
    void clearCache();
    void releaseCache();
    virtual std::shared_ptr<Datagram> makeDatagram(const size_t length);

    // ──────── STATUS CONTROL ────────
public Q_SLOTS:
    void onRestart();
    void onStart();
    void onStop();

public:
    void initialize(quint64 watchdog,
        QString rxAddress,
        quint16 rxPort,
        quint16 txPort,
        bool separateRxTxSocket,
        const std::set<QString>& multicastGroup,
        const std::set<QString>& multicastListeningInterfaces,
        const std::set<QString>& multicastOutgoingInterfaces,
        bool inputEnabled,
        bool multicastLoopback);

Q_SIGNALS:
    void isBoundedChanged(const bool isBounded);
    void socketError(int error, const QString description);

    void multicastGroupJoined(QString group, QString interfaceName);
    void multicastGroupLeaved(QString group, QString interfaceName);

public Q_SLOTS:
    void setWatchdogTimeout(const quint64 ms);

    // Unicast/Multicast - Input

    void setAddress(const QString& address);
    void setRxPort(const quint16 port);
    void setInputEnabled(const bool enabled);

    // Unicast/Multicast - Output

    void setTxPort(const quint16 port);

    // Multicast - Input

    void joinMulticastGroup(const QString& address);
    void leaveMulticastGroup(const QString& address);

    // Join every addresses in '_multicastGroups' on interface with name 'interfaceName'
    void joinMulticastInterface(const QString& interfaceName);
    // Leave every addresses in '_multicastGroups' on interface with name 'interfaceName'
    void leaveMulticastInterface(const QString& interfaceName);

    void setMulticastLoopback(const bool loopback);

    // Multicast - Output

    void setMulticastOutgoingInterfaces(const QStringList& interfaces);

    // Create a different socket for unicast rx and multicast tx
    void setSeparateRxTxSockets(const bool separateRxTxSocketsChanged);

private:
    void tryJoinAllAvailableInterfaces();
    void tryLeaveAllAvailableInterfaces();

    bool joinAndTrackMulticastGroup(const QString& address, const QString& interfaceName);

    bool socketJoinMulticastGroup(const QString& address, const QString& interfaceName);
    bool socketLeaveMulticastGroup(const QString& address, const QString& interfaceName);

    void setMulticastLoopbackToSocket() const;
    void startWatchdog();
    void stopWatchdog();
    void setMulticastTtl(const quint8 ttl);

    // ──────── MULTICAST INTERFACE JOIN WATCHER ────────
private:
    // Created when at least one interface is being joined. ie (!_joinedMulticastGroups.empty() || !_failedJoiningMulticastGroup.empty())
    // Destroy in 'onStop', when _joinedMulticastGroups & _failedJoiningMulticastGroup are both empty
    QTimer* _listeningMulticastInterfaceWatcher = nullptr;

    void startListeningMulticastInterfaceWatcher();
    void stopListeningMulticastInterfaceWatcher();

    // ──────── MULTICAST TX JOIN WATCHER ────────
private:
    // Created when
    QTimer* _outputMulticastInterfaceWatcher = nullptr;
    QElapsedTimer _txMulticastPacketElapsedTime;

    void startOutputMulticastInterfaceWatcher();
    void stopOutputMulticastInterfaceWatcher();

    void createMulticastSocketForInterface(const IInterface& interface);

    void createMulticastOutputSockets();
    void destroyMulticastOutputSockets();

Q_SIGNALS:
    // Private signal
    void queueStartWatchdog();

    // ──────── TX ────────
public Q_SLOTS:
    virtual void onSendDatagram(const SharedDatagram& datagram);

    // ──────── RX ────────
protected:
    virtual bool isPacketValid(const uint8_t* buffer, const size_t length) const;
private Q_SLOTS:
    void readPendingDatagrams();

protected:
    virtual void onDatagramReceived(const SharedDatagram& datagram);
Q_SIGNALS:
    void datagramReceived(const SharedDatagram datagram);

    // ──────── STATUS ────────
protected Q_SLOTS:
    void onSocketError(QAbstractSocket::SocketError error);
    void onRxSocketError(QAbstractSocket::SocketError error);
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);

private:
    void onSocketErrorCommon(QAbstractSocket::SocketError error, QUdpSocket* socket);

private:
    static QString socketStateToString(QAbstractSocket::SocketState socketState);

    // ──────── COUNTER ────────
private:
    quint64 _rxBytesCounter = 0;
    quint64 _txBytesCounter = 0;
    quint64 _rxPacketsCounter = 0;
    quint64 _txPacketsCounter = 0;
    quint64 _rxInvalidPacket = 0;
    QTimer* _bytesCounterTimer = nullptr;

protected:
    void startBytesCounter();
    virtual void stopBytesCounter();

Q_SIGNALS:
    void rxBytesCounterChanged(const quint64 rx);
    void txBytesCounterChanged(const quint64 tx);
    void rxPacketsCounterChanged(const quint64 rx);
    void txPacketsCounterChanged(const quint64 tx);
    void rxInvalidPacketsCounterChanged(const quint64 rx);
};

}

#endif // __NETUDP_SERVER_WORKER_HPP__
