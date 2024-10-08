// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __NETUDP_SERVER_HPP__
#define __NETUDP_SERVER_HPP__

#include <NetUdp/Export.hpp>
#include <NetUdp/Property.hpp>
#include <NetUdp/Datagram.hpp>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#ifdef NETUDP_ENABLE_QML
#    include <QtQml/QJSValue>
#endif
#include <memory>

namespace netudp {

class Worker;

class NETUDP_API_ ISocket : public QObject
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    ISocket(QObject* parent = nullptr);

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
    // You need to subclass netudp::Worker to have any benefit.
    NETUDP_PROPERTY(bool, useWorkerThread, UseWorkerThread);

    // ──────── ATTRIBUTE MULTICAST INPUT ────────
protected:
    // List of all multicast group the socket is listening to
    // You can use joinMulticastGroup and leaveMulticastGroup api to join or leave an interface
    // leaveAllMulticastGroups will leave all the multicast groups
    // isMulticastGroupPresent will return true if an address is present in this list
    NETUDP_ABSTRACT_PROPERTY(QStringList, multicastGroups, MulticastGroups);

    // Incoming interfaces for multicast packets that are going to be listened.
    // If empty, then every interfaces available on your system that support multicast are going to be listened.
    NETUDP_ABSTRACT_PROPERTY(QStringList, multicastListeningInterfaces, MulticastListeningInterfaces);

    // ──────── ATTRIBUTE MULTICAST OUTPUT ────────
protected:
    // Outgoing interfaces for multicast packet
    // Is empty, every packet is send to every interfaces.
    // Example Unix: eth0, lo, wifi0 ...
    //  - to get them run 'ifconfig'
    // Example Windows: ethernet_67890, loopback_0, wifi_12345
    //  - to get them run 'Get-NetAdapter -Name "*" -IncludeHidden | Format-List -Property "Name", "InterfaceDescription", "InterfaceName"'
    NETUDP_PROPERTY(QStringList, multicastOutgoingInterfaces, MulticastOutgoingInterfaces);

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

#ifdef NETUDP_ENABLE_QML
    // Example:
    // ```js
    // socket.sendDatagram({
    //   data: "datagram as string",
    //   address: "127.0.0.1",
    //   port: 1234,
    //   ttl: 8
    // })
    // ```
    //
    virtual bool sendDatagram(QJSValue datagram) = 0;
#endif

public:
    virtual bool sendDatagram(
        const uint8_t* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl = 0) = 0;
    virtual bool sendDatagram(
        const char* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl = 0) = 0;
    virtual bool sendDatagram(std::shared_ptr<Datagram> datagram, const QString& address, const uint16_t port, const uint8_t ttl = 0) = 0;
    virtual bool sendDatagram(std::shared_ptr<Datagram> datagram) = 0;

    // ──────── SIGNALS ────────
Q_SIGNALS:
    void socketError(int error, const QString description);

    void multicastGroupJoined(QString group, QString interfaceName);
    void multicastGroupLeaved(QString group, QString interfaceName);

    void sharedDatagramReceived(const SharedDatagram& datagram);
#ifdef NETUDP_ENABLE_QML
    void datagramReceived(QJSValue datagram);
#endif
};

struct SocketPrivate;

class NETUDP_API_ Socket : public ISocket
{
    Q_OBJECT
    NETUDP_REGISTER_TO_QML(Socket);

    // ──────── CONSTRUCTOR ────────
public:
    Socket(QObject* parent = nullptr);
    ~Socket() override;

    // ──────── WORKER ────────
protected:
    // Kill worker from main thread
    // Set _worker & _workerThread to nullptr
    void killWorker();

public:
    bool setUseWorkerThread(const bool& enabled) override;

    QStringList multicastGroups() const override;
    bool setMulticastGroups(const QStringList& value) override;

    QStringList multicastListeningInterfaces() const override;
    bool setMulticastListeningInterfaces(const QStringList& values) override;

    // ──────── C++ API ────────
public Q_SLOTS:
    bool start() override;
    bool start(quint16 port) override final;
    bool start(const QString& address, quint16 port) override final;
    bool restart() override final;
    bool stop() override;

    bool joinMulticastGroup(const QString& groupAddress) override final;
    bool leaveMulticastGroup(const QString& groupAddress) override final;
    bool leaveAllMulticastGroups() override final;
    bool isMulticastGroupPresent(const QString& groupAddress) override final;

    bool joinMulticastInterface(const QString& name) override final;
    bool leaveMulticastInterface(const QString& name) override final;
    bool leaveAllMulticastInterfaces() override final;
    bool isMulticastInterfacePresent(const QString& name) override final;

    void clearRxCounter() override final;
    void clearTxCounter() override final;
    void clearRxInvalidCounter() override final;
    void clearCounters() override final;

    // ──────── CUSTOM WORKER API ────────
protected:
    virtual Worker* createWorker();

    // ──────── CUSTOM DATAGRAM API ────────
public:
    virtual std::shared_ptr<Datagram> makeDatagram(const size_t length);

    // ──────── SEND DATAGRAM API ────────
public:
    bool sendDatagram(
        const uint8_t* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl = 0) override;
    bool sendDatagram(const char* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl = 0) override;
    bool sendDatagram(std::shared_ptr<Datagram> datagram, const QString& address, const uint16_t port, const uint8_t ttl = 0) override;
    bool sendDatagram(std::shared_ptr<Datagram> datagram) override;
#ifdef NETUDP_ENABLE_QML
    bool sendDatagram(QJSValue datagram) override;
#endif

    bool isSendDatagramAllowed() const;

    // ──────── RECEIVE DATAGRAM API ────────
protected Q_SLOTS:
    // If overriding this function, you should also emit 'datagramReceived'
    virtual void onDatagramReceived(const SharedDatagram& datagram);

    // ──────── PRIVATE WORKER COMMUNICATION (FROM) ────────
private Q_SLOTS:
    void onWorkerRxPerSecondsChanged(const quint64 rxBytes);
    void onWorkerTxPerSecondsChanged(const quint64 txBytes);
    void onWorkerPacketsRxPerSecondsChanged(const quint64 rxPackets);
    void onWorkerPacketsTxPerSecondsChanged(const quint64 txPackets);
    void onWorkerRxInvalidPacketsCounterChanged(const quint64 rxPackets);

    // ──────── PRIVATE WORKER COMMUNICATION (TO) ────────
Q_SIGNALS:
    void startWorker();
    void stopWorker();
    void restartWorker();
    void joinMulticastGroupWorker(const QString address);
    void leaveMulticastGroupWorker(const QString address);
    void joinMulticastInterfaceWorker(const QString address);
    void leaveMulticastInterfaceWorker(const QString address);
    void sendDatagramToWorker(netudp::SharedDatagram datagram);

private:
    std::unique_ptr<SocketPrivate> _p;
};

}

#endif
