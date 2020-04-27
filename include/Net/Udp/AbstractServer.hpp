#ifndef __NETUDP_ABSTRACT_SERVER_HPP__
#define __NETUDP_ABSTRACT_SERVER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Export.hpp>
#include <Net/Udp/Property.hpp>

// Qt
#include <QObject>

// C++ Header
#include <set>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net {
namespace udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ IAbstractServer : public QObject
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    IAbstractServer(QObject* parent = nullptr) : QObject(parent) {}

    // ──────── ATTRIBUTE STATE ────────
protected:
    NETUDP_PROPERTY_RO(bool, isRunning, Running);
    NETUDP_PROPERTY_RO(bool, isBounded, Bounded);
    NETUDP_PROPERTY_D(quint64, watchdogPeriod, WatchdogPeriod, 5000);

    // ──────── ATTRIBUTE ────────
protected:
    NETUDP_PROPERTY(QString, rxAddress, RxAddress);
    NETUDP_PROPERTY(quint16, rxPort, RxPort);
    NETUDP_PROPERTY(quint16, txPort, TxPort);
    NETUDP_PROPERTY(bool, separateRxTxSockets, SeparateRxTxSockets);
    NETUDP_PROPERTY(bool, inputEnabled, InputEnabled);
    NETUDP_PROPERTY(bool, useWorkerThread, UseWorkerThread);

    // ──────── ATTRIBUTE MULTICAST ────────
protected:
    NETUDP_ABSTRACT_PROPERTY(QList<QString>, multicastGroups, MulticastGroups);
    NETUDP_PROPERTY(QString, multicastInterfaceName, MulticastInterfaceName);
    NETUDP_PROPERTY(bool, multicastLoopback, MulticastLoopback);

    // ──────── STATUS ────────
protected:
    NETUDP_PROPERTY_RO(quint64, rxBytesPerSeconds, RxBytesPerSeconds);
    NETUDP_PROPERTY_RO(quint64, txBytesPerSeconds, TxBytesPerSeconds);
    NETUDP_PROPERTY_RO(quint64, rxBytesTotal, RxBytesTotal);
    NETUDP_PROPERTY_RO(quint64, txBytesTotal, TxBytesTotal);

    NETUDP_PROPERTY_RO(quint64, rxPacketsPerSeconds, RxPacketsPerSeconds);
    NETUDP_PROPERTY_RO(quint64, txPacketsPerSeconds, TxPacketsPerSeconds);
    NETUDP_PROPERTY_RO(quint64, rxPacketsTotal, RxPacketsTotal);
    NETUDP_PROPERTY_RO(quint64, txPacketsTotal, TxPacketsTotal);

    NETUDP_PROPERTY_RO(quint64, rxInvalidPacketTotal, RxInvalidPacketTotal);

    // ──────── C++ API ────────
public Q_SLOTS:
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool restart() = 0;

    virtual bool joinMulticastGroup(const QString& groupAddress) = 0;
    virtual bool leaveMulticastGroup(const QString& groupAddress) = 0;
    virtual bool leaveAllMulticastGroups() = 0;
    virtual bool isMulticastGroupPresent(const QString& groupAddress) = 0;

    virtual void clearRxCounter() = 0;
    virtual void clearTxCounter() = 0;
    virtual void clearRxInvalidCounter() = 0;
    virtual void clearCounters() = 0;

    // ──────── SIGNALS ────────
Q_SIGNALS:
    void socketError(int error, const QString description);
};

class NETUDP_API_ AbstractServer : public IAbstractServer
{
    Q_OBJECT
    NETUDP_REGISTER_TO_QML(AbstractServer);

    // ──────── CONSTRUCTOR ────────
public:
    AbstractServer(QObject* parent = nullptr);

    // ──────── MULTICAST ────────
private:
    std::set<QString> _multicastGroups;

protected:
    const std::set<QString>& multicastGroupsSet() const;

public:
    bool setMulticastGroups(const QList<QString>& value) override;
    QList<QString> multicastGroups() const override;
    bool setMulticastInterfaceName(const QString& name) override;

    // ──────── C++ API ────────
public Q_SLOTS:
    bool start() override;
    bool stop() override;
    bool restart() override;

    bool joinMulticastGroup(const QString& groupAddress) override;
    bool leaveMulticastGroup(const QString& groupAddress) override;
    bool leaveAllMulticastGroups() override;
    bool isMulticastGroupPresent(const QString& groupAddress) override;

    void clearRxCounter() override;
    void clearTxCounter() override;
    void clearRxInvalidCounter() override;
    void clearCounters() override;
};

}
}

#endif
