#ifndef __NETUDP_ABSTRACT_SERVER_HPP__
#define __NETUDP_ABSTRACT_SERVER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header
#include <cstdint>
#include <set>

// Qt Header
#include <QObject>

// Dependencies Header
#include <QQmlAutoPropertyHelpers.h>

// Application Header
#include <NetUdp/Export.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

NETUDP_NAMESPACE_START

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

    // ──────── ATTRIBUTE INPUT ────────
protected:
    QSM_WRITABLE_AUTO_PROPERTY(QString, address, Address);
    QSM_WRITABLE_AUTO_PROPERTY(quint16, port, Port);

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

    // ──────── ATTRIBUTE STATUS ────────
protected:
    QSM_WRITABLE_AUTO_PROPERTY(quint64, rxBytesPerSeconds, RxBytesPerSeconds);
    QSM_WRITABLE_AUTO_PROPERTY(quint64, txBytesPerSeconds, TxBytesPerSeconds);
    QSM_WRITABLE_AUTO_PROPERTY(quint64, rxBytesTotal, RxBytesTotal);
    QSM_WRITABLE_AUTO_PROPERTY(quint64, txBytesTotal, TxBytesTotal);

public:
    Q_INVOKABLE virtual bool start();
    Q_INVOKABLE virtual bool stop();
    Q_INVOKABLE virtual bool restart();

    Q_INVOKABLE virtual bool joinMulticastGroup(const QString& groupAddress);

    Q_INVOKABLE virtual bool leaveMulticastGroup(const QString& groupAddress);

    bool isMulticastGroupPresent(const QString& groupAddress);
};

NETUDP_NAMESPACE_END

#endif // __NETUDP_ABSTRACT_SERVER_HPP__
