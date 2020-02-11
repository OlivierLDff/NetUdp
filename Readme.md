# NetUdp

NetUdp provide a Udp Server that can send and receive Datagram.

## Overview

![ClassDiagram](./doc/ClassDiagram.svg)

### Introduction

The two main classes that work out of the box are `Server` and `RecycledDatagram`. Simply create a server, start it. Then send and receive datagrams. The server can join multicast group to receice multicast packets.

The `Server` use a `ServerWorker` that can run on separate thread or in main thread.

Every datagram allocation is stored in `std::shared_ptr<Datagram>`. This allow to reuse datagram object structure already allocated later without reallocating anything.

* `AbstractServer` can be inherited to represent a `Server` without any functionality.

* `Server` and `ServerWorker` can be inherited to implement custom communication between server and worker. For example sending custom objects that can be serialized/deserialized in worker thread.

* `Datagram` can be inherited if a custom data container if required. For example if data is already serialized in a structure. Putting a reference to that structure inside the `Datagram` avoid a copy to `RecycledDatagram`.

### Dependencies

* The library depends on C++ 14 STL.
* [Recycler](https://github.com/OlivierLDff/Recycler.git) library to reuse allocated datagram.
* [Stringify](https://github.com/OlivierLDff/Stringify) for version to string and qml regex.
* Qt Core and Network for the backend.
* Qt Qml Quick Control2 for Qml Debugging.
* [Qaterial](https://github.com/OlivierLDff/Qaterial) for qml debugging.

### Tools

* [CMake](https://cmake.org/) v3.14 or greater.
* C++14 compliant compiler or greater.
* Internet connection to download dependencies from *Github* during configuration.

### Out of the box usage

A Basic Client/Server can be found in `examples/EchoClientServer.cpp`.

#### Server

This example demonstrate how to create a server that send datagram to address `127.0.0.1` on port `9999`.

```cpp
#include <NetUdp.hpp>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    Net::Udp::Server server;
    server.start();
    const std::string data = "Dummy Data";
    server.sendDatagram(data.c_str(), data.length()+1, "127.0.0.1", 9999);

    return QCoreApplication::exec();
}
```

> The datagram is emitted from a random port chosen by the operating system. It can be explicitly specified by calling `setTxPort(uint16_t)`.
>
> If the server also receive datagram (ie `inputEnabled` is true and call `setRxPort`), then the rx port will use. To change this default behavior call `setSeparateRxTxSockets(true)`.

#### Client

This example demonstrate how to receive a packet on address `127.0.0.1` on port `9999`.

```c++
#include <NetUdp.hpp>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    Net::Udp::Server client;
    client.setRxAddress("127.0.0.1");
    client.setRxPort(9999);
    client.start();

        QObject::connect(&client, &Net::Udp::Server::datagramReceived,
                         [](const Net::Udp::SharedDatagram& d)
        {
            qInfo("Rx : %s", reinterpret_cast<const char*>(d->buffer()));
        });


    return QCoreApplication::exec();
}
```

#### Errors handling

Errors can be observed via `socketError(int error, QString description)` signals. If the socket fail to bind, or if anything happened, the worker will start a watchdog timer to restart the socket.

The default restart time is set to 5 seconds but can be changed via `watchdogPeriodMs` property.

#### Disable Input

By default, if internal socket is bounded to an interface with a port, the `Worker` will receive incoming datagram. To avoid receiving those datagram inside `Server`, call `setInputEnabled(false)`.

#### Multicast group

It's really to join or leave multicast ip. Simply call `joinMulticastGroup(QString)` or `leaveMulticastGroup(QString)`/`leaveAllMulticastGroups`. All joined multicast group can be retrieve with `multicastGroups`.

By default multicast packet are not received is listening to datagram. To enable this behavior call `setMulticastLoopback(true)`.

#### Statistics

Internally the `Server` track multiple information to have an idea of what is going on.

* `isBounded` indicate if the socket is currently binded to a network interface.
* `*xBytesPerSeconds` is an average value of all bytes received/sent in the last second. This value is updated every seconds. `* can be replaced by t and r`
* `*xBytesTotal` total received/sent bytes since start. `* can be replaced by t and r`
* `*xPacketsPerSeconds` is an average value of all packets received/sent in the last second. This value is updated every seconds. `* can be replaced by t and r`
* `*xPacketsTotal` total received/sent packets since start. `* can be replaced by t and r`

Those indicate can be cleared with `clearRxCounter`/`clearTxCounter`/`clearCounters`.

#### How to avoid memory copy

When calling any of the following function, a `memcpy` will happen to a `RecycledDatagram`.

```cpp
virtual bool sendDatagram(const uint8_t* buffer, const size_t length, const QHostAddress& address, const uint16_t port, const uint8_t ttl = 0);
virtual bool sendDatagram(const uint8_t* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl = 0);
virtual bool sendDatagram(const char* buffer, const size_t length, const QHostAddress& address, const uint16_t port, const uint8_t ttl = 0);
virtual bool sendDatagram(const char* buffer, const size_t length, const QString& address, const uint16_t port, const uint8_t ttl = 0);
```

To avoid useless memory copy it's recommended to retrieve a datagram from `Server` cache with `makeDatagram(const size_t length)`. Then use this `Net::Udp::SharedDatagram` to serialize data. And call :

```cpp
virtual bool sendDatagram(std::shared_ptr<Datagram> datagram, const QString& address, const uint16_t port, const uint8_t ttl = 0);
virtual bool sendDatagram(std::shared_ptr<Datagram> datagram);
```

### Customize AbstractServer

If you are not satisfied by `Server` behavior, or if you want to mock `Server` without any dependency to `QtNetwork`. It's possible to extend `AbstractServer` to use it's basic functionality.

* Managing list of multicast ip.
* start/stop behavior that clear counter and `isRunning`/`isBounded`.

You need to override:

* `bool start()` : Start the server. Auto restart to survive from error is expected. Don't forget to call `AbstractServer::start` at beginning.
* `bool stop()` : Stop the server. Clear all running task, empty cache, buffers, etc... Don't forget to call `AbstractServer::stop` at beginning. To ensure maximum cleaning, always stop every even if stopping any part failed.
* `joinMulticastGroup(const QString& groupAddress)`: Implementation to join a multicast group. Don't forget to call `AbstractServer::joinMulticastGroup`.
* `leaveMulticastGroup(const QString& groupAddress)`: Implementation to leave a multicast group. Don't forget to call `AbstractServer::leaveMulticastGroup`.

```cpp
#include <Net/Udp/AbstractServer.hpp>

class MyAbstractServer : Net::Udp::AbstractServer
{
    Q_OBJECT
public:
    MyAbstractServer(QObject* parent = nullptr) : Net::Udp::AbstractServer(parent) {}

public Q_SLOTS:
    bool start() override
    {
        if(!Net::Udp::AbstractServer::start())
            return false;

        // Do your business ...

        return true;
    }
    bool stop() override
    {
        auto stopped = Net::Udp::AbstractServer::stop()

        // Do your business ...

        return stopped;
    }
    bool joinMulticastGroup(const QString& groupAddress) override
    {
        // Join groupAddress ...
        return true;
    }
    bool leaveMulticastGroup(const QString& groupAddress) override
    {
        // Leave groupAddress ...
        return true;
    }
}
```



### Customize Server and Worker

`Server` and `Worker` mainly work in pair, so if overriding one, it make often sense to override the other.

Reasons to override `Worker`:

* Implement a serialization/deserialization in a worker thread.
* Check if a datagram is valid with computation of crc, hash, etc... on every received datagram in worker thread.
* Compute crc, hash, ... for every outgoing datagram in worker thread.
* Use a custom `Datagram` class
* ...

Reasons to override `Server`

* Use a custom `Worker` class.
* Use a custom `Datagram` class.
* ...

#### Customize Datagram

Using a custom `Datagram` can reduce memory copy depending on your application.

* To use custom datagram for Rx packet, customize `Worker`.
* To use custom datagram for Tx packet:
  * Call `Server::sendDatagram(SharedDatagram, ...)` with it.
  * Customize `Server` to use it when calling with `Server::sendDatagram(const uint8_t*, ...)`.  *A `memcpy` will happen. So don't use a custom `Datagram` for that purpose.*

```cpp
#include <Net/Udp/Datagram.hpp>

class MyDatagram : Net::Udp::Datagram
{
    uint8_t* myBuffer = nullptr;
    size_t myLength = 0;
public:
    uint8_t* buffer() { return myBuffer; }
    const uint8_t* buffer() const { return myBuffer; }
    size_t length() const { return myLength; }
};
```

#### Customize ServerWorker

When inheriting from `ServerWorker` you can override:

* `bool isPacketValid(const uint8_t* buffer, const size_t length) const`: Called each time a datagram is received. Check if a packet is valid depending on your protocol. Default implementation just return true. You can add a CRC check or something like that. Returning false here will increment the `rxInvalidPacketTotal` counter in `Server`.
* `void onReceivedDatagram(const SharedDatagram& datagram)`: Called each time a valid datagram arrive. Default implementation emit `receivedDatagram` signal. Override this function to add a custom messaging system, or a custom deserialization.
* `std::shared_ptr<Datagram> makeDatagram(const size_t length)` : Create custom `Datagram` for rx.
* If you implement a custom serialization via a custom message system in `Worker`, call `void onSendDatagram(const SharedDatagram& datagram)` to send a datagram to the network.
* Don't forget that `ServerWorker` inherit from `QObject`, so use `Q_OBJECT` macro to generate custom signals.

Example:

```cpp
#include <Net/Udp/ServerWorker.hpp>

class MyServerWorker : Net::Udp::ServerWorker
{
    Q_OBJECT
public:
    MyServerWorker(QObject* parent = nullptr) : Net::Udp::ServerWorker(parent) {}

public Q_SLOTS:
    bool std::unique_ptr<ServerWorker> createWorker() override
    {
        auto myWorker = std::make_unique<MyWorker>();

        // Init your worker with custom stuff ...
        // Even keep reference to MyWorker* if you need later access

        // It's recommended to communicate via signals to the worker
        // Connect here ...

        return std::move(myWorker);
    }

    void onDatagramReceived(const SharedDatagram& datagram) override
    {
        // Do your business ...

        // This super call is optionnal. If not done Server will never trigger onDatagramReceived
        Net::Udp::ServerWorker::onDatagramReceived(datagram);
    }

    std::shared_ptr<Datagram> makeDatagram(const size_t length) override
    {
        // Return your custom diagram type used for rx
        return std::make_shared<MyDiagram>(length);
    }
}
```

> Customizing worker mostly make sense when it's running in a separate thread. Otherwise it won't give any performance boost. Don't forget to call `Server::setUseWorkerThread(true)`.
>

#### Customize Server

When inheriting from `Server` you can override:

* `bool std::unique_ptr<ServerWorker> createWorker() const`: Create a custom worker.
* `void onDatagramReceived(const SharedDatagram& datagram)` : Handle datagram in there. Default implementation emit `datagramReceived` signals
* `std::shared_ptr<Datagram> makeDatagram(const size_t length)` : Create custom `Datagram` that will be used in `Server::sendDatagram(const uint8_t*, ...)`.
* Don't forget that `Server` inherit from `QObject`, so use `Q_OBJECT` macro to generate custom signals.

Example:

```cpp
#include <Net/Udp/Server.hpp>

class MyServer : Net::Udp::Server
{
    Q_OBJECT
public:
    MyServer(QObject* parent = nullptr) : Net::Udp::Server(parent) {}

public Q_SLOTS:
    bool std::unique_ptr<ServerWorker> createWorker() override
    {
        auto myWorker = std::make_unique<MyWorker>();

        // Init your worker with custom stuff ...
        // Even keep reference to MyWorker* if you need later access

        // It's recommended to communicate via signals to the worker
        // Connect here ...

        return std::move(myWorker);
    }

    void onDatagramReceived(const SharedDatagram& datagram) override
    {
        // Do your business ...

        // This super call is optionnal. If not done Server will never trigger datagramReceived signal
        Net::Udp::Server::onDatagramReceived(datagram);
    }

    std::shared_ptr<Datagram> makeDatagram(const size_t length) override
    {
        // Return your custom diagram type used for tx
        return std::make_shared<MyDiagram>(length);
    }
}
```

## Examples

### EchoClientServer

This example demonstrate an echo between a server and a client. Server send a packet to a client, the client reply the same packet. `Ctrl+C` to quit.

```bash
$> NetUdp_EchoClientServer --help

Options:
  -?, -h, --help    Displays this help.
  -t                Make the worker live in a different thread. Default false
  -s, --src <port>  Port for rx packet. Default "11111".
  -d, --dst <port>  Port for tx packet. Default "11112".
  --src-addr <ip>   Ip address for server. Default "127.0.0.1"
  --dst-addr <ip>   Ip address for client. Default "127.0.0.1"

$> NetUdp_EchoClientServer
> app: Init application
> server: Set Rx Address to 127.0.0.1
> server: Set Rx Port to 11111
> client: Set Rx Address to 127.0.0.1
> client: Set Rx Port to 11112
> app: Start application
> client: Rx : Echo 0
> server: Rx : Echo 0
> client: Rx : Echo 1
> server: Rx : Echo 1
> client: Rx : Echo 2
> server: Rx : Echo 2
> ...
```

### MulticastLoopbackServer

Demonstrate how to join multicast ip group. Send a packet and read it back via loopback.

```bash
$> NetUdp_EchoMulticastLoopback --help
Options:
  -?, -h, --help          Displays this help.
  -t                      Make the worker live in a different thread. Default
                          false
  -p                      Print available multicast interface name
  -s, --src <port>        Port for rx packet. Default "11111".
  -i, --ip <ip>           Ip address of multicast group. Default "239.0.0.1"
  --if, --interface <if>  Name of the iface to join. Default is os dependent

```

## Qml Debug

This library also provide a tool object that demonstrate every Qmls functionality. This is intended for quick debug, or test functionalities if UI isn't built yet.

![Qml](./doc/Qml.png)

In order to use this qml object into another qml file, multiple steps are required.

* Call `Net::Udp::Utils::registerTypes(...)` to register `AbstractServer`, `Server`, `SharedDatagram`, ... to the qml system
* Call `Net::Udp::Utils::loadResources()` to load every `NetUdp` resources into the `qrc`.

Then simply to something like that:

```js
import NetUdp.Debug 1.0 as NetUdpDebug
import NetUdp 1.0 as NetUdp

Rectangle
{
    property NetUdp.Server server
	NetUdpDebug.Server
    {
        object: server
    }
}
```

`NetUdp.Debug.Server` is a `Qaterial.DebugObject`. If you want the raw content to display it somewhere else, then use `NetUdp.Debug.ServerContent` that is a `Column`.

## Configuring

This library use CMake for configuration.

```bash
git clone https://github.com/OlivierLDff/NetUdp
cd NetUdp
mkdir build && cd build
cmake ..
```

The `CMakeLists.txt` will download every dependencies for you.

## Building

Simply use integrated cmake command:

```bash
cmake --build . --config "Release"
```

### Execute Examples

## Integrating

Adding NetUdp library in your library is really simple if you use CMake 3.14.

In your `CMakeLists.txt`:

```cmake
# ...
include(FetchContent)
FetchContent_Declare(
    NetUdp
    GIT_REPOSITORY "https://github.com/OlivierLDff/NetUdp"
    GIT_TAG        "master"
)
# ...
FetchContent_MakeAvailable(NetUdp)
# ...

target_link_libraries(MyTarget NetUdp)
```

Then you just need to `#include <NetUdp.hpp>`.

## Authors

* [Olivier Le Doeuff](https://github.com/OlivierLDff)