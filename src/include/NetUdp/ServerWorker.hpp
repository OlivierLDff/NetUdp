#ifndef __NETUDP_SERVER_WORKER_HPP__
#define __NETUDP_SERVER_WORKER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header
#include <QObject>

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

    // ──────── ATTRIBUTE ────────
private:
    std::unique_ptr<QUdpSocket> _socket;
    std::unique_ptr<QTimer> _watchdog;
    bool _isBounded = false;
    quint64 _watchdogTimeout = 5000;
    QString _address;
    quint16 _port = 0;
    QMap<QString, bool> _multicastGroups;
    QString _multicastInterfaceName;
    bool _multicastLoopback = false;
    quint8 _multicastTtl = 0;

    // ──────── STATUS CONTROL ────────
public slots:
    void onRestart();
    void onStart();
    void onStop();

signals:
    void isBoundedChanged(const bool isBounded);
    void socketError(int error, const QString description);

public slots:
    void setWatchdogTimeout(const quint64 ms);
    void setAddress(const QString address);
    void setPort(const quint16 port);
    void joinMulticastGroup(const QString address);
    void leaveMulticastGroup(const QString address);
    void setMulticastInterfaceName(const QString name);
    void setMulticastLoopback(const bool loopback);

private:
    void setMulticastInterfaceNameToSocket() const;
    void setMulticastLoopbackToSocket() const;
    void startWatchdog() const;
    void stopWatchdog() const;
    void setMulticastTtl(const quint8 ttl);

    // ──────── TX ────────
public slots:
    virtual void onSendDatagram(SharedDatagram datagram);

    // ──────── RX ────────
protected:
    virtual bool isPacketValid(const uint8_t* buffer, const size_t length) const;

private slots:
    void readPendingDatagrams();

    // ──────── STATUS ────────
public slots:
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onWatchdogTimeout();

private:
    static QString socketStateToString(QAbstractSocket::SocketState socketState);

    // ──────── STATISTICS ────────
private:
    quint64 _rxBytesCounter = 0;
    quint64 _txBytesCounter = 0;
    std::unique_ptr<QTimer> _bytesCounterTimer;

private:
    void startBytesCounter();
    void stopBytesCounter();

private slots:
    void updateDataCounter();
signals:
    void rxBytesCounterChanged(const quint64 rx);
    void txBytesCounterChanged(const quint64 tx);

    // ──────── FRIENDS ────────

    friend class Server;
};



NETUDP_NAMESPACE_END

#endif // __NETUDP_SERVER_WORKER_HPP__
