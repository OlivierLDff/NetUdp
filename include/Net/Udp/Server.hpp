#ifndef __NETUDP_SERVER_HPP__
#define __NETUDP_SERVER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/AbstractServer.hpp>
#include <Net/Udp/RecycledDatagram.hpp>
#include <Net/Udp/Export.hpp>

// Dependencies Header
#include <Recycler/Circular.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Udp {

class ServerWorker;

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ Server : public AbstractServer
{
    Q_OBJECT
    QSM_REGISTER_TO_QML(Server);

    // ──────── CONSTRUCTOR ────────
public:
    Server(QObject* parent = nullptr);
    ~Server();

    // ──────── WORKER ────────
protected:
    std::unique_ptr<ServerWorker> _worker;
    std::unique_ptr<QThread> _workerThread;
    Recycler::Circular<RecycledDatagram> _cache;

    // ──────── INPUT / OUTPUT ────────
protected:
    Q_PROPERTY(bool inputEnabled READ inputEnabled WRITE setInputEnabled NOTIFY inputEnabledChanged);
private:
    bool _inputEnabled = true;
public:
    bool inputEnabled() const;
    void setInputEnabled(const bool enabled);

Q_SIGNALS:
    void inputEnabledChanged(bool enabled);

    // ──────── WORKER THREAD USE ────────
protected:
    Q_PROPERTY(bool useWorkerThread READ useWorkerThread WRITE setUseWorkerThread NOTIFY useWorkerThreadChanged);
private:
    bool _useWorkerThread = false;
public:
    bool useWorkerThread() const;
    void setUseWorkerThread(const bool enabled);

Q_SIGNALS:
    void useWorkerThreadChanged(bool enabled);

    // ──────── C++ API ────────
public Q_SLOTS:
    bool start() override;
    bool stop() override;
    bool joinMulticastGroup(const QString& groupAddress) override final;
    bool leaveMulticastGroup(const QString& groupAddress) override final;

protected:
    virtual std::unique_ptr<ServerWorker> createWorker();

    // ──────── DATAGRAM API ────────
public:
    virtual std::shared_ptr<Datagram> makeDatagram(const size_t length);

    virtual bool sendDatagram(const uint8_t* buffer, const size_t length, const QHostAddress& address, const uint16_t port, const uint8_t ttl = 0);
    virtual bool sendDatagram(const uint8_t* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl = 0);
    virtual bool sendDatagram(const char* buffer, const size_t length, const QHostAddress& address, const uint16_t port, const uint8_t ttl = 0);
    virtual bool sendDatagram(const char* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl = 0);
    virtual bool sendDatagram(std::shared_ptr<Datagram> datagram, const QString& address, const uint16_t port, const uint8_t ttl = 0);
    virtual bool sendDatagram(std::shared_ptr<Datagram> datagram);

protected Q_SLOTS:
    virtual void onDatagramReceived(const SharedDatagram& datagram);
Q_SIGNALS:
    void datagramReceived(const SharedDatagram& datagram);

    // ──────── WORKER COMMUNICATION ────────
Q_SIGNALS:
    void startWorker();
    void stopWorker();
    void restartWorker();
    void joinMulticastGroupWorker(const QString address);
    void leaveMulticastGroupWorker(const QString address);
    void sendDatagramToWorker(SharedDatagram datagram);

private Q_SLOTS:
    void onBoundedChanged(const bool isBounded);
    void onWorkerRxPerSecondsChanged(const quint64 rxBytes);
    void onWorkerTxPerSecondsChanged(const quint64 txBytes);
    void onWorkerPacketsRxPerSecondsChanged(const quint64 rxPackets);
    void onWorkerPacketsTxPerSecondsChanged(const quint64 txPackets);
    void onWorkerRxInvalidPacketsCounterChanged(const quint64 rxPackets);
};

}
}

#endif // __NETUDP_SERVER_HPP__
