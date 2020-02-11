#ifndef __NETUDP_SERVER_WORKER_HPP__
#define __NETUDP_SERVER_WORKER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Headers
#include <Net/Udp/RecycledDatagram.hpp>
#include <Net/Udp/Export.hpp>

// Dependencies Headers
#include <Recycler/Circular.hpp>

// Qt Headers
#include <QObject>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QTimer>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ ServerWorker: public QObject
{
    Q_OBJECT

    // ──────── CONSTRUCTOR ────────
public:
    ServerWorker(QObject* parent = nullptr);

    // ──────── ATTRIBUTE ────────
private:
    std::unique_ptr<QUdpSocket> _socket;
    std::unique_ptr<QUdpSocket> _rxSocket;
    std::unique_ptr<QTimer> _watchdog;
    Recycler::Circular<RecycledDatagram> _cache;
    bool _isBounded = false;
    quint64 _watchdogTimeout = 5000;
    QString _rxAddress;
    quint16 _rxPort = 0;
    quint16 _txPort = 0;
    QMap<QString, bool> _multicastGroups;
    QNetworkInterface _multicastInterface;
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
    QMap<QString, bool> multicastGroups() const;
    QNetworkInterface multicastInterface() const;
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

Q_SIGNALS:
    void isBoundedChanged(const bool isBounded);
    void socketError(int error, const QString description);

public Q_SLOTS:
    void setWatchdogTimeout(const quint64 ms);
    void setAddress(const QString& address);
    void setRxPort(const quint16 port);
    void setTxPort(const quint16 port);
    void joinMulticastGroup(const QString& address);
    void leaveMulticastGroup(const QString& address);
    void setMulticastInterfaceName(const QString& name);
    void setMulticastLoopback(const bool loopback);
    void setInputEnabled(const bool enabled);
    void setSeparateRxTxSockets(const bool separateRxTxSocketsChanged);

private:
    void setMulticastInterfaceNameToSocket() const;
    void setMulticastLoopbackToSocket() const;
    void startWatchdog() const;
    void stopWatchdog() const;
    void setMulticastTtl(const quint8 ttl);

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
    void onWatchdogTimeout();

private:
    static QString socketStateToString(QAbstractSocket::SocketState socketState);

    // ──────── STATISTICS ────────
private:
    quint64 _rxBytesCounter = 0;
    quint64 _txBytesCounter = 0;
    quint64 _rxPacketsCounter = 0;
    quint64 _txPacketsCounter = 0;
    quint64 _rxInvalidPacket = 0;
    std::unique_ptr<QTimer> _bytesCounterTimer;

protected:
    void startBytesCounter();
    virtual void stopBytesCounter();

protected Q_SLOTS:
    virtual void updateDataCounter();
Q_SIGNALS:
    void rxBytesCounterChanged(const quint64 rx);
    void txBytesCounterChanged(const quint64 tx);
    void rxPacketsCounterChanged(const quint64 rx);
    void txPacketsCounterChanged(const quint64 tx);
    void rxInvalidPacketsCounterChanged(const quint64 rx);

    // ──────── FRIENDS ────────
    friend class Server;
};

}
}

#endif // __NETUDP_SERVER_WORKER_HPP__
