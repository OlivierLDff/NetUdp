#ifndef __NETUDP_SERVER_HPP__
#define __NETUDP_SERVER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header
#include <QObject>
#include <QThread>

// Dependencies Header

// Application Header
#include <NetUdp/Export.hpp>
#include <NetUdp/Datagram.hpp>
#include <NetUdp/AbstractServer.hpp>
#include <NetUdp/ServerWorker.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

NETUDP_NAMESPACE_START

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ Server : public AbstractServer
{
    Q_OBJECT
    QSM_REGISTER_OBJ_TO_QML_NO_NAME(Server);

    // ──────── CONSTRUCTOR ────────
public:
    Server(QObject* parent = nullptr);
    ~Server();

    // ──────── WORKER ────────
protected:
    std::unique_ptr<ServerWorker> _worker;
    std::unique_ptr<QThread> _workerThread;

    // ──────── C++ API ────────
public:
    Q_INVOKABLE bool start() override;
    Q_INVOKABLE bool stop() override;
    Q_INVOKABLE bool joinMulticastGroup(const QString& groupAddress) override final;
    Q_INVOKABLE bool leaveMulticastGroup(const QString& groupAddress) override final;
    virtual std::unique_ptr<ServerWorker> createWorker() const;

    virtual bool sendDatagram(uint8_t* buffer, const size_t length, const QHostAddress& address, const uint16_t port, const uint8_t ttl = 0);
    virtual bool sendDatagram(std::shared_ptr<Datagram> datagram);

    // ──────── WORKER COMMUNICATION ────────
Q_SIGNALS:
    void startWorker();
    void stopWorker();
    void joinMulticastGroupWorker(const QString address);
    void leaveMulticastGroupWorker(const QString address);
    void sendDatagramToWorker(SharedDatagram datagram);

private Q_SLOTS:
    void onBoundedChanged(const bool isBounded);
    void onWorkerRxPerSecondsChanged(const quint64 rxBytes);
    void onWorkerTxPerSecondsChanged(const quint64 txBytes);
};

NETUDP_NAMESPACE_END

#endif // __NETUDP_SERVER_HPP__
