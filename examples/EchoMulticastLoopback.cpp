
// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Dependencies
#include <Stringify.hpp>
#include <NetUdp.hpp>

// Qt
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QTimer>
#include <QNetworkInterface>

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

    Net::Udp::Server server;

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
            server.setMulticastInterfaceName(iface);
        }

        server.setUseWorkerThread(multiThreaded);
        server.setMulticastLoopback(true);

        QObject::connect(&timer, &QTimer::timeout, [this]()
            {
                const std::string data = "Echo " + std::to_string(counter++);
                server.sendDatagram(data.c_str(), data.length()+1, QHostAddress(ip), port);
            });

        QObject::connect(&server, &Net::Udp::Server::datagramReceived, [](const Net::Udp::SharedDatagram& d)
            {
                qCInfo(SERVER_LOG_CAT, "Rx : %s", reinterpret_cast<const char*>(d->buffer()));
            });

        qCInfo(APP_LOG_CAT, "Start application");

        QObject::connect(&server, &Net::Udp::Server::isRunningChanged, [](bool value)
            {
                qCInfo(SERVER_LOG_CAT, "isRunning : %d", signed(value));
            });
        QObject::connect(&server, &Net::Udp::Server::isBoundedChanged, [](bool value)
            {
                qCInfo(SERVER_LOG_CAT, "isBounded : %d", signed(value));
            });
        QObject::connect(&server, &Net::Udp::Server::socketError, [](int value, const QString error)
            {
                qCInfo(SERVER_LOG_CAT, "error : %s", qPrintable(error));
            });

        server.start();
        timer.start(1000);
    }
};

int main(int argc, char* argv[])
{
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

    QCommandLineOption portOption(QStringList() << "s" << "src",
        QCoreApplication::translate("main", "Port for rx packet. Default \"11111\"."),
        QCoreApplication::translate("main", "port"));
    parser.addOption(portOption);
    portOption.setDefaultValue("11111");

    QCommandLineOption ipOption(QStringList() << "i" << "ip",
        QCoreApplication::translate("main", "Ip address of multicast group. Default \"239.0.0.1\""),
        QCoreApplication::translate("main", "ip"));
    parser.addOption(ipOption);

    QCommandLineOption ifOption(QStringList() << "if" << "interface",
        QCoreApplication::translate("main", "Name of the iface to join. Default is os dependent"),
        QCoreApplication::translate("main", "if"));
    parser.addOption(ifOption);
    ifOption.setDefaultValue("");

    // Process the actual command line arguments given by the user
    parser.process(app);

    if(parser.isSet(printIfaceName))
    {
        qCInfo(APP_LOG_CAT, "Available multicast interface name : ");
        for(const auto& iface : QNetworkInterface::allInterfaces())
        {
            if(iface.flags() & QNetworkInterface::CanMulticast)
                qCInfo(APP_LOG_CAT, "%s", qPrintable(iface.name()));
        }
        return 0;
    }
    
    // ────────── APPLICATION ──────────────────────────────────────

    // Register types for to use SharedDatagram in signals
    Net::Udp::Utils::registerTypes();

    // Create the app and start it
    App echo;
    bool ok;
    const auto port = parser.value(portOption).toInt(&ok);
    if (ok)
        echo.port = port;
    const auto ip = parser.value(ipOption);
    if (!ip.isEmpty())
        echo.ip = ip;
    const auto iface = parser.value(ifOption);
    if (!iface.isEmpty())
        echo.iface = iface;
    echo.multiThreaded = parser.isSet(multiThreadOption);

    echo.start();

    // Start event loop
    return QCoreApplication::exec();
}
