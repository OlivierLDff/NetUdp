#ifndef __NETUDP_SERVER_WORKER_HPP__
#define __NETUDP_SERVER_WORKER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header
#include <QObject>
#include <QNetworkInterface>

// Dependencies Header

// Application Header
#include <NetUdp/Export.hpp>
#include <NetUdp/Datagram.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

class QUdpSocket;
class QTimer;

NETUDP_NAMESPACE_START

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ ServerWorker: public QObject
{
    Q_OBJECT

    // ──────── CONSTRUCTOR ────────
public:
    ServerWorker(QObject* parent = nullptr);
    ~ServerWorker();

    // ──────── ATTRIBUTE ────────
private:
    std::unique_ptr<QUdpSocket> _socket;
    std::unique_ptr<QTimer> _watchdog;
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

private:
    void setMulticastInterfaceNameToSocket() const;
    void setMulticastLoopbackToSocket() const;
    void startWatchdog() const;
    void stopWatchdog() const;
    void setMulticastTtl(const quint8 ttl);

    // ──────── TX ────────
public Q_SLOTS:
    virtual void onSendDatagram(SharedDatagram datagram);

    // ──────── RX ────────
protected:
    virtual bool isPacketValid(const uint8_t* buffer, const size_t length);

private Q_SLOTS:
    void readPendingDatagrams();
protected:
    virtual void onReceivedDatagram(const SharedDatagram& datagram);
Q_SIGNALS:
    void receivedDatagram(const SharedDatagram datagram);

    // ──────── STATUS ────────
public Q_SLOTS:
    void onSocketError(QAbstractSocket::SocketError error);
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

    // ──────── FRIENDS ────────

    friend class Server;
};


NETUDP_NAMESPACE_END

#endif // __NETUDP_SERVER_WORKER_HPP__
