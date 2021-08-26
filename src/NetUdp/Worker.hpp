// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __NETUDP_SERVER_WORKER_HPP__
#define __NETUDP_SERVER_WORKER_HPP__

#include <NetUdp/Export.hpp>
#include <NetUdp/Datagram.hpp>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNetwork/QAbstractSocket>

QT_FORWARD_DECLARE_CLASS(QUdpSocket);

#include <set>
#include <memory>

namespace netudp {

struct WorkerPrivate;

class NETUDP_API_ Worker : public QObject
{
    Q_OBJECT

    // ──────── CONSTRUCTOR ────────
public:
    Worker(QObject* parent = nullptr);
    ~Worker();

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

    void startListeningMulticastInterfaceWatcher();
    void stopListeningMulticastInterfaceWatcher();
    // ──────── MULTICAST TX JOIN WATCHER ────────
private:
    void startOutputMulticastInterfaceWatcher();
    void stopOutputMulticastInterfaceWatcher();

    void createMulticastSocketForInterface(const IInterface& iface);

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
protected:
    void startBytesCounter();
    virtual void stopBytesCounter();

Q_SIGNALS:
    void rxBytesCounterChanged(const quint64 rx);
    void txBytesCounterChanged(const quint64 tx);
    void rxPacketsCounterChanged(const quint64 rx);
    void txPacketsCounterChanged(const quint64 tx);
    void rxInvalidPacketsCounterChanged(const quint64 rx);

private:
    std::unique_ptr<WorkerPrivate> _p;
};

}

#endif // __NETUDP_SERVER_WORKER_HPP__
