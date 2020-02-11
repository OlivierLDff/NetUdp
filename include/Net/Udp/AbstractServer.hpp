#ifndef __NETUDP_ABSTRACT_SERVER_HPP__
#define __NETUDP_ABSTRACT_SERVER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────


// Application Header
#include <Net/Udp/Export.hpp>

// Dependencies Header
#include <QQmlAutoPropertyHelpers.h>

// Qt Header
#include <QObject>

// C++ Header
#include <set>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ AbstractServer : public QObject
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    AbstractServer(QObject* parent = nullptr);

    // ──────── ATTRIBUTE STATE ────────
protected:
    QSM_READONLY_AUTO_PROPERTY(bool, isRunning, IsRunning);
    QSM_READONLY_AUTO_PROPERTY(bool, isBounded, IsBounded);
    QSM_WRITABLE_AUTO_PROPERTY_WDEFAULT(quint64, watchdogPeriodMs, WatchdogPeriodMs, 5000);

Q_SIGNALS:
    void socketError(int error, const QString description);

    // ──────── ATTRIBUTE ────────
protected:
    QSM_WRITABLE_AUTO_PROPERTY(QString, rxAddress, RxAddress);
    QSM_WRITABLE_AUTO_PROPERTY(quint16, rxPort, RxPort);
    QSM_WRITABLE_AUTO_PROPERTY(quint16, txPort, TxPort);
    QSM_WRITABLE_AUTO_PROPERTY(bool, separateRxTxSockets, SeparateRxTxSockets);

    // ──────── ATTRIBUTE MULTICAST ────────
protected:
    Q_PROPERTY(QList<QString> multicastGroups READ multicastGroups NOTIFY multicastGroupsChanged)
private:
    std::set<QString> _multicastGroups;
protected:
    const std::set<QString>& multicastGroupsSet() const;
public:
    QList<QString> multicastGroups() const;
Q_SIGNALS:
    void multicastGroupsChanged();

protected:
    Q_PROPERTY(QString multicastInterfaceName READ multicastInterfaceName WRITE setMulticastInterfaceName NOTIFY multicastInterfaceNameChanged);
private:
    QString _multicastInterfaceName;
public:
    QString multicastInterfaceName() const;
    bool setMulticastInterfaceName(const QString& name);
Q_SIGNALS:
    void multicastInterfaceNameChanged(const QString name);

protected:
    QSM_WRITABLE_AUTO_PROPERTY(bool, multicastLoopback, MulticastLoopback);

    // ──────── STATUS ────────
protected:
    QSM_READONLY_AUTO_PROPERTY(quint64, rxBytesPerSeconds, RxBytesPerSeconds);
    QSM_READONLY_AUTO_PROPERTY(quint64, txBytesPerSeconds, TxBytesPerSeconds);
    QSM_READONLY_AUTO_PROPERTY(quint64, rxBytesTotal, RxBytesTotal);
    QSM_READONLY_AUTO_PROPERTY(quint64, txBytesTotal, TxBytesTotal);

    QSM_READONLY_AUTO_PROPERTY(quint64, rxPacketsPerSeconds, RxPacketsPerSeconds);
    QSM_READONLY_AUTO_PROPERTY(quint64, txPacketsPerSeconds, TxPacketsPerSeconds);
    QSM_READONLY_AUTO_PROPERTY(quint64, rxPacketsTotal, RxPacketsTotal);
    QSM_READONLY_AUTO_PROPERTY(quint64, txPacketsTotal, TxPacketsTotal);

    QSM_READONLY_AUTO_PROPERTY(quint64, rxInvalidPacketTotal, RxInvalidPacketTotal);

    // ──────── C++ API ────────
public Q_SLOTS:
    virtual bool start();
    virtual bool stop();
    virtual bool restart();

    virtual bool joinMulticastGroup(const QString& groupAddress);
    virtual bool leaveMulticastGroup(const QString& groupAddress);
    bool leaveAllMulticastGroups();
    bool isMulticastGroupPresent(const QString& groupAddress);

    void clearRxCounter();
    void clearTxCounter();
    void clearRxInvalidCounter();
    void clearCounters();
};

}
}

#endif // __NETUDP_ABSTRACT_SERVER_HPP__