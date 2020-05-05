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

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net {
namespace udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ ISocket : public QObject
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    ISocket(QObject* parent = nullptr) : QObject(parent) {}

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
    NETUDP_PROPERTY_D(bool, inputEnabled, InputEnabled, true);
    NETUDP_PROPERTY(bool, useWorkerThread, UseWorkerThread);

    // ──────── ATTRIBUTE MULTICAST ────────
protected:
    NETUDP_ABSTRACT_PROPERTY(QStringList, multicastGroups, MulticastGroups);
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
    virtual bool start(quint16 port) = 0;
    virtual bool start(const QString& address, quint16 port) = 0;
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

}
}

#endif
