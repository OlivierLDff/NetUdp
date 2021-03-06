﻿
// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Dependencies
#include <Net/Udp/NetUdp.hpp>

// spdlog
#ifdef WIN32
#    include <spdlog/sinks/msvc_sink.h>
#endif
#include <spdlog/sinks/stdout_color_sinks.h>

// Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QCommandLineParser>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkInterface>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(APP_LOG_CAT, "app")
Q_LOGGING_CATEGORY(SERVER_LOG_CAT, "server")
Q_LOGGING_CATEGORY(CLIENT_LOG_CAT, "client")

class App
{
public:
    int counter = 0;

    uint16_t port = 11111;
    QString ip = QStringLiteral("239.0.0.1");
    QString iface;

    net::udp::Socket server;

    bool multiThreaded = false;

    QTimer timer;

public:
    void start()
    {
        qCInfo(APP_LOG_CAT, "Init application");

        qCInfo(SERVER_LOG_CAT, "Join multicast group %s", qPrintable(ip));
        server.joinMulticastGroup(ip);
        qCInfo(SERVER_LOG_CAT, "Set Rx Port to %d", signed(port));
        server.setRxPort(port);
        if(!iface.isEmpty())
        {
            qCInfo(SERVER_LOG_CAT, "Set Multicast Interface Name to %s", qPrintable(iface));
            server.setMulticastOutgoingInterfaces({iface});
        }

        server.setUseWorkerThread(multiThreaded);
        server.setMulticastLoopback(true);

        QObject::connect(&timer,
            &QTimer::timeout,
            [this]()
            {
                const std::string data = "Echo " + std::to_string(counter++);
                server.sendDatagram(data.c_str(), data.length() + 1, ip, port);
            });

        QObject::connect(&server,
            &net::udp::Socket::sharedDatagramReceived,
            [](const net::udp::SharedDatagram& d) { qCInfo(SERVER_LOG_CAT, "Rx : %s", reinterpret_cast<const char*>(d->buffer())); });

        qCInfo(APP_LOG_CAT, "Start application");

        QObject::connect(&server,
            &net::udp::Socket::isRunningChanged,
            [](bool value) { qCInfo(SERVER_LOG_CAT, "isRunning : %d", signed(value)); });
        QObject::connect(&server,
            &net::udp::Socket::isBoundedChanged,
            [](bool value) { qCInfo(SERVER_LOG_CAT, "isBounded : %d", signed(value)); });
        QObject::connect(&server,
            &net::udp::Socket::socketError,
            [](int value, const QString error) { qCInfo(SERVER_LOG_CAT, "error : %s", qPrintable(error)); });

        server.start();
        timer.start(1000);
    }
};

int main(int argc, char* argv[])
{
#ifdef WIN32
    const auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvcSink->set_level(spdlog::level::debug);
    net::udp::Logger::registerSink(msvcSink);
#endif

    const auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdoutSink->set_level(spdlog::level::debug);
    net::udp::Logger::Logger::registerSink(stdoutSink);

    QCoreApplication app(argc, argv);

    // ────────── COMMAND PARSER ──────────────────────────────────────

    QCommandLineParser parser;
    parser.setApplicationDescription("Echo Client Server");
    parser.addHelpOption();

    QCommandLineOption multiThreadOption(QStringList() << "t",
        QCoreApplication::translate("main", "Make the worker live in a different thread. Default false"));
    parser.addOption(multiThreadOption);

    QCommandLineOption printIfaceName(QStringList() << "p",
        QCoreApplication::translate("main", "Print available multicast interface name"));
    parser.addOption(printIfaceName);

    QCommandLineOption portOption(QStringList() << "s"
                                                << "src",
        QCoreApplication::translate("main", "Port for rx packet. Default \"11111\"."),
        QCoreApplication::translate("main", "port"));
    parser.addOption(portOption);
    portOption.setDefaultValue("11111");

    QCommandLineOption ipOption(QStringList() << "i"
                                              << "ip",
        QCoreApplication::translate("main", "Ip address of multicast group. Default \"239.0.0.1\""),
        QCoreApplication::translate("main", "ip"));
    parser.addOption(ipOption);

    QCommandLineOption ifOption(QStringList() << "if"
                                              << "interface",
        QCoreApplication::translate("main", "Name of the iface to join. Default is os dependent"),
        QCoreApplication::translate("main", "if"));
    parser.addOption(ifOption);
    ifOption.setDefaultValue("");

    // Process the actual command line arguments given by the user
    parser.process(app);

    if(parser.isSet(printIfaceName))
    {
        qCInfo(APP_LOG_CAT, "Available multicast interface name : ");
        for(const auto& iface: QNetworkInterface::allInterfaces())
        {
            if(iface.flags() & QNetworkInterface::CanMulticast)
                qCInfo(APP_LOG_CAT, "%s", qPrintable(iface.name()));
        }
        return 0;
    }

    // ────────── APPLICATION ──────────────────────────────────────

    // Register types for to use SharedDatagram in signals
    net::udp::registerQmlTypes();

    // Create the app and start it
    App echo;
    bool ok;
    const auto port = parser.value(portOption).toInt(&ok);
    if(ok)
        echo.port = port;
    const auto ip = parser.value(ipOption);
    if(!ip.isEmpty())
        echo.ip = ip;
    const auto iface = parser.value(ifOption);
    if(!iface.isEmpty())
        echo.iface = iface;
    echo.multiThreaded = parser.isSet(multiThreadOption);

    echo.start();

    // Start event loop
    return QCoreApplication::exec();
}
