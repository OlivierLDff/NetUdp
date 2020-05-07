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
    // Set to true when start is called, false when stop is called.
    NETUDP_PROPERTY_RO(bool, isRunning, Running);

    // Indicate if socket is correctly bounded
    // You shouldn't try to send any datagram if the socket isn't bounded.
    NETUDP_PROPERTY_RO(bool, isBounded, Bounded);

    // If bind failed, or if a network error occur, the socket will restart in watchdogPeriod (ms)
    NETUDP_PROPERTY_D(quint64, watchdogPeriod, WatchdogPeriod, 5000);

    // ──────── ATTRIBUTE ────────
protected:
    // Address to listen, if not set it will listen on any address.
    // This address can be ipv4 in the form A.B.C.D
    // Or ipv6 in the form 12:23:45:67:89...
    NETUDP_PROPERTY(QString, rxAddress, RxAddress);

    // Port an which to listen for incoming packets.
    // This is the same port for multicast and unicast.
    NETUDP_PROPERTY(quint16, rxPort, RxPort);

    // Specify a tx port. This is optional/
    // If not specified decision is done:
    // !inputEnabled && separateRxTxSockets:
    //  decision is os dependent
    // else
    //  port will be the same as rxPort.
    NETUDP_PROPERTY(quint16, txPort, TxPort);

    // Can be set to ensure that txPort is different that listening port (decision for the os if txPort is 0)
    NETUDP_PROPERTY(bool, separateRxTxSockets, SeparateRxTxSockets);

    // Can be disabled to avoid receiving any datagram.
    NETUDP_PROPERTY_D(bool, inputEnabled, InputEnabled, true);

    // Move the worker into a worker thread.
    // This allow to move in the worker serialization/deserialization/encoding/... work
    // You need to subclass net::udp::Worker to have any benefit.
    NETUDP_PROPERTY(bool, useWorkerThread, UseWorkerThread);

    // ──────── ATTRIBUTE MULTICAST ────────
protected:
    // List of all multicast group the socket is listening to
    // You can use joinMulticastGroup and leaveMulticastGroup api to join or leave an interface
    // leaveAllMulticastGroups will leave all the multicast groups
    // isMulticastGroupPresent will return true if an address is present in this list
    NETUDP_ABSTRACT_PROPERTY(QStringList, multicastGroups, MulticastGroups);

    // Incoming interfaces for multicast packets
    // If empty, then it will be os dependent.
    // If you want to listen for multicast packet on every interface, then set multicastListenOnAllInterfaces to true
    // This property have no effect if multicastListenOnAllInterfaces is set.
    NETUDP_ABSTRACT_PROPERTY(
        QStringList, multicastListeningInterfaces, MulticastListeningInterfaces);

    // Bypass multicastListeningInterfaces if set, and the worker will join multicast groups
    // on every interfaces that support multicast.
    NETUDP_PROPERTY_D(bool, multicastListenOnAllInterfaces, MulticastListenOnAllInterfaces, true);

    // Outgoing interface for multicast packet
    // Is empty, decision is os dependent.
    // Example Unix: eth0, lo, wifi0 ...
    //  - to get them run ifconfig
    // Example Windows: ethernet_67890, loopback_0, wifi_12345
    //  - to get them run Get-NetAdapter -Name "*" -IncludeHidden | Format-List -Property "Name", "InterfaceDescription", "InterfaceName"
    NETUDP_PROPERTY(QString, multicastInterfaceName, MulticastInterfaceName);

    // Set the IP_MULTICAST_LOOP option
    // Windows: Should be set on receiver
    // Linux: Should be set on sender
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

    virtual bool joinMulticastInterface(const QString& name) = 0;
    virtual bool leaveMulticastInterface(const QString& name) = 0;
    virtual bool leaveAllMulticastInterfaces() = 0;
    virtual bool isMulticastInterfacePresent(const QString& name) = 0;

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
