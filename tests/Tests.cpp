// ────── INCLUDE ───────

#include <NetUdp/NetUdp.hpp>
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <gtest/gtest.h>
#include <string>
#include <cstring>

namespace netudp {

class UnicastClientServer : public ::testing::Test
{
protected:
    void init()
    {
        rx.setRxAddress(serverListeningAddr);
        rx.setRxPort(serverListeningPort);
    }

    uint16_t serverListeningPort = 11111;
    QString serverListeningAddr = QStringLiteral("127.0.0.1");

    netudp::Socket rx;
    netudp::Socket tx;

    void start()
    {
        QSignalSpy spyServerBounded(&rx, &Socket::isBoundedChanged);
        QSignalSpy spyClientBounded(&tx, &Socket::isBoundedChanged);

        rx.start();
        tx.start();

        ASSERT_TRUE(tx.isRunning());
        ASSERT_TRUE(rx.isRunning());

        if(!rx.isBounded())
            ASSERT_TRUE(spyServerBounded.wait(5000));
        if(!tx.isBounded())
            ASSERT_TRUE(spyClientBounded.wait(5000));

        ASSERT_TRUE(tx.isBounded());
        ASSERT_TRUE(rx.isBounded());
    }

    void clientToServerTest()
    {
        QSignalSpy spy(&rx, &Socket::sharedDatagramReceived);

        start();

        const std::string sentString = "My datagram packet";
        tx.sendDatagram(sentString.c_str(), sentString.length(), serverListeningAddr, serverListeningPort);

        if(spy.empty())
            ASSERT_TRUE(spy.wait(5000));

        const auto arguments = spy.takeFirst(); // take the first signal
        ASSERT_FALSE(arguments.isEmpty());
        const auto datagram = qvariant_cast<netudp::SharedDatagram>(arguments.at(0));
        ASSERT_NE(datagram, nullptr);

        const std::string receivedString(reinterpret_cast<const char*>(datagram->buffer()), datagram->length());
        ASSERT_EQ(receivedString, sentString);
    }
};

TEST_F(UnicastClientServer, clientToServer)
{
    serverListeningPort = 1111;
    init();
    rx.setUseWorkerThread(false);
    tx.setUseWorkerThread(false);
    clientToServerTest();
}

TEST_F(UnicastClientServer, clientToServerMultithread)
{
    serverListeningPort = 1112;
    init();
    rx.setUseWorkerThread(true);
    tx.setUseWorkerThread(true);
    clientToServerTest();
}

// Server send multicast data to client
class MulticastClientServer : public ::testing::Test
{
protected:
    void init()
    {
        rx.setMulticastGroups({multicastGroup});
        rx.setRxPort(multicastPort);

        // Windows: Should be set on receiver
        // Linux: Should be set on sender
        tx.setMulticastLoopback(true);
        rx.setMulticastLoopback(true);
    }

    uint16_t multicastPort = 11112;
    QString multicastGroup = QStringLiteral("239.1.2.3");

    netudp::Socket tx;
    netudp::Socket rx;

    void start()
    {
        QSignalSpy spyTxBounded(&tx, &Socket::isBoundedChanged);
        QSignalSpy spyRxBounded(&rx, &Socket::isBoundedChanged);

        tx.start();
        rx.start();

        ASSERT_TRUE(rx.isRunning());
        ASSERT_TRUE(tx.isRunning());

        if(!tx.isBounded())
            ASSERT_TRUE(spyTxBounded.wait(5000));
        if(!rx.isBounded())
            ASSERT_TRUE(spyRxBounded.wait(5000));

        ASSERT_TRUE(rx.isBounded());
        ASSERT_TRUE(tx.isBounded());
    }

    void serverToClientTest()
    {
        QSignalSpy spy(&rx, &Socket::sharedDatagramReceived);

        start();
        // Wait one second to be sure subscription succeed
        QTest::qWait(1000);

        const std::string sentString = "My Multicast datagram packet";
        tx.sendDatagram(sentString.c_str(), sentString.length(), multicastGroup, multicastPort);

        if(spy.empty())
            ASSERT_TRUE(spy.wait(5000));

        const auto arguments = spy.takeFirst(); // take the first signal
        ASSERT_FALSE(arguments.isEmpty());
        const auto datagram = qvariant_cast<netudp::SharedDatagram>(arguments.at(0));
        ASSERT_NE(datagram, nullptr);

        const std::string receivedString(reinterpret_cast<const char*>(datagram->buffer()), datagram->length());
        ASSERT_EQ(receivedString, sentString);
    }
};

TEST_F(MulticastClientServer, Sync)
{
    multicastPort = 1113;
    init();
    tx.setUseWorkerThread(false);
    tx.setUseWorkerThread(false);
    serverToClientTest();
}

TEST_F(MulticastClientServer, Async)
{
    multicastPort = 1114;
    init();
    tx.setUseWorkerThread(true);
    tx.setUseWorkerThread(true);
    serverToClientTest();
}

// Server send multicast data to client
class MulticastClient2Server : public ::testing::Test
{
protected:
    uint16_t multicastPort = 11234;
    QString multicastGroup = QStringLiteral("239.4.5.6");

    netudp::Socket tx;
    netudp::Socket rx1;
    netudp::Socket rx2;

    void start()
    {
        rx1.setMulticastGroups({multicastGroup});
        rx1.setRxPort(multicastPort);

        rx2.setRxPort(multicastPort);

        // Windows: Should be set on receiver
        // Linux: Should be set on sender
        tx.setMulticastLoopback(true);
        rx1.setMulticastLoopback(true);
        rx2.setMulticastLoopback(true);

        QSignalSpy spyTxBounded(&tx, &Socket::isBoundedChanged);
        QSignalSpy spyRx1Bounded(&rx1, &Socket::isBoundedChanged);
        QSignalSpy spyRx2Bounded(&rx2, &Socket::isBoundedChanged);

        tx.start();
        rx1.start();
        rx2.start();

        rx2.setMulticastGroups({multicastGroup});

        ASSERT_TRUE(tx.isRunning());
        ASSERT_TRUE(rx1.isRunning());
        ASSERT_TRUE(rx2.isRunning());

        if(!tx.isBounded())
            ASSERT_TRUE(spyTxBounded.wait(5000));
        if(!rx1.isBounded())
            ASSERT_TRUE(spyRx1Bounded.wait(5000));
        if(!rx2.isBounded())
            ASSERT_TRUE(spyRx2Bounded.wait(5000));

        ASSERT_TRUE(tx.isBounded());
        ASSERT_TRUE(rx1.isBounded());
        ASSERT_TRUE(rx2.isBounded());
    }

    void serverToClientTest()
    {
        QSignalSpy spy1(&rx1, &Socket::sharedDatagramReceived);
        QSignalSpy spy2(&rx2, &Socket::sharedDatagramReceived);

        start();

        // Wait one second to be sure subscription succeed
        QTest::qWait(1000);

        const std::string sentString = "My Multicast datagram packet";
        tx.sendDatagram(sentString.c_str(), sentString.length(), multicastGroup, multicastPort);

        if(spy1.empty())
            ASSERT_TRUE(spy1.wait(5000));

        const auto arguments1 = spy1.takeFirst();
        ASSERT_FALSE(arguments1.isEmpty());
        const auto datagram1 = qvariant_cast<netudp::SharedDatagram>(arguments1.at(0));
        ASSERT_NE(datagram1, nullptr);
        const std::string receivedString1(reinterpret_cast<const char*>(datagram1->buffer()), datagram1->length());
        ASSERT_EQ(receivedString1, sentString);

        if(spy2.empty())
            ASSERT_TRUE(spy2.wait(5000));

        const auto arguments2 = spy2.takeFirst();
        ASSERT_FALSE(arguments2.isEmpty());
        const auto datagram2 = qvariant_cast<netudp::SharedDatagram>(arguments2.at(0));
        ASSERT_NE(datagram2, nullptr);
        const std::string receivedString2(reinterpret_cast<const char*>(datagram2->buffer()), datagram2->length());
        ASSERT_EQ(receivedString2, sentString);
    }
};

TEST_F(MulticastClient2Server, Sync)
{
    multicastPort = 11235;
    tx.setUseWorkerThread(false);
    tx.setUseWorkerThread(false);
    serverToClientTest();
}

TEST_F(MulticastClient2Server, Async)
{
    multicastPort = 11236;
    tx.setUseWorkerThread(true);
    tx.setUseWorkerThread(true);
    serverToClientTest();
}

TEST(WorkerMultithreadFuzz, restart)
{
    Socket client;
    client.setUseWorkerThread(true);

#ifdef NDEBUG
    const auto CREATION_COUNT = 10;
#else
    const auto CREATION_COUNT = 10;
#endif

    for(int i = 0; i < CREATION_COUNT; ++i)
    {
        client.start(1234);
        client.stop();
    }

    for(int i = 0; i < CREATION_COUNT; ++i)
    {
        QSignalSpy spyClientBounded(&client, &Socket::isBoundedChanged);
        client.start(1234);
        if(!client.isBounded())
            ASSERT_TRUE(spyClientBounded.wait(1000));
        client.stop();
    }

    for(int i = 0; i < CREATION_COUNT; ++i)
    {
        QSignalSpy spyClientBounded(&client, &Socket::isBoundedChanged);
        client.start(1234);
        char data[100];
        client.sendDatagram(data, 100, "127.0.0.1", 1234);
        if(!client.isBounded())
            ASSERT_TRUE(spyClientBounded.wait(1000));
        client.stop();
    }
}

TEST(WorkerMultithreadFuzz, restartPointer)
{
    auto client = new Socket();
    client->setUseWorkerThread(true);

#ifdef NDEBUG
    const auto CREATION_COUNT = 100;
#else
    const auto CREATION_COUNT = 10;
#endif

    for(int i = 0; i < CREATION_COUNT; ++i)
    {
        QSignalSpy spyClientBounded(client, &Socket::isBoundedChanged);
        client->start(1234);
        char data[100];
        client->sendDatagram(data, 100, "127.0.0.1", 1234);
        if(!client->isBounded())
            ASSERT_TRUE(spyClientBounded.wait(1000));
        delete client;
        client = new Socket();
    }

    delete client;
}

TEST(WorkerMultithreadFuzz, releaseBindedPort)
{
#ifdef NDEBUG
    const auto CREATION_COUNT = 100;
    ;
#else
    const auto CREATION_COUNT = 10;
#endif

    for(int i = 0; i < CREATION_COUNT; ++i)
    {
        Socket client;
        Socket server;
        client.setUseWorkerThread(true);
        server.setUseWorkerThread(true);
        QSignalSpy spyServerBounded(&server, &Socket::isBoundedChanged);
        QSignalSpy spyClientBounded(&client, &Socket::isBoundedChanged);
        QSignalSpy spyDatagramReceived(&client, &Socket::sharedDatagramReceived);

        client.start(1234);
        server.start();

        if(spyServerBounded.empty())
        {
            qDebug() << "Waiting for server bounded";
            ASSERT_TRUE(spyServerBounded.wait());
        }

        if(spyClientBounded.empty())
        {
            qDebug() << "Waiting for client bounded";
            ASSERT_TRUE(spyClientBounded.wait());
        }

        auto d = server.makeDatagram(10);
        std::memset(d->buffer(), 0x12, 10);
        server.sendDatagram(d, "127.0.0.1", 1234);

        qDebug() << "Waiting for datagram";
        ASSERT_TRUE(spyDatagramReceived.wait());
    }
}

}

int main(int argc, char** argv)
{
    // An application is required to wait for signals with QSignalSpy
    QCoreApplication application(argc, argv);

    // Register custom type to work with signals
    netudp::registerQmlTypes();

    // Start tests
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
