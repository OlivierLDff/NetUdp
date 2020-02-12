
// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Dependencies
#include <NetUdp.hpp>

// Qt
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QTimer>

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

    uint16_t src = 11111;
    uint16_t dst = 11112;

    QString srcAddr = QStringLiteral("127.0.0.1");
    QString dstAddr = QStringLiteral("127.0.0.1");

    Net::Udp::Server server;
    Net::Udp::Server client;

    bool multiThreaded = false;

    QTimer timer;

public:
    void start()
    {
        qCInfo(APP_LOG_CAT, "Init application");

        qCInfo(SERVER_LOG_CAT, "Set Rx Address to %s", qPrintable(srcAddr));
        server.setRxAddress(srcAddr);
        qCInfo(SERVER_LOG_CAT, "Set Rx Port to %d", signed(src));
        server.setRxPort(src);

        qCInfo(CLIENT_LOG_CAT, "Set Rx Address to %s", qPrintable(dstAddr));
        client.setRxAddress(dstAddr);
        qCInfo(CLIENT_LOG_CAT, "Set Rx Port to %d", signed(dst));
        client.setRxPort(dst);

        server.setUseWorkerThread(multiThreaded);
        client.setUseWorkerThread(multiThreaded);

        QObject::connect(&timer, &QTimer::timeout, [this]()
            {
                const std::string data = "Echo " + std::to_string(counter++);
                server.sendDatagram(data.c_str(), data.length()+1, QHostAddress(dstAddr), dst);
            });

        QObject::connect(&server, &Net::Udp::Server::datagramReceived, [](const Net::Udp::SharedDatagram& d)
            {
                qCInfo(SERVER_LOG_CAT, "Rx : %s", reinterpret_cast<const char*>(d->buffer()));
            });

        QObject::connect(&client, &Net::Udp::Server::datagramReceived, [this](const Net::Udp::SharedDatagram& d)
            {
                qCInfo(CLIENT_LOG_CAT, "Rx : %s", reinterpret_cast<const char*>(d->buffer()));
                client.sendDatagram(d->buffer(), d->length(), QHostAddress(srcAddr), src);
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
        QObject::connect(&client, &Net::Udp::Server::isRunningChanged, [](bool value)
            {
                qCInfo(CLIENT_LOG_CAT, "isRunning : %d", signed(value));
            });
        QObject::connect(&client, &Net::Udp::Server::isBoundedChanged, [](bool value)
            {
                qCInfo(CLIENT_LOG_CAT, "isBounded : %d", signed(value));
            });
        QObject::connect(&client, &Net::Udp::Server::socketError, [](int value, const QString error)
            {
                qCInfo(CLIENT_LOG_CAT, "error : %s", qPrintable(error));
            });


        server.start();
        client.start();
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

    QCommandLineOption srcPortOption(QStringList() << "s" << "src",
        QCoreApplication::translate("main", "Port for rx packet. Default \"11111\"."),
        QCoreApplication::translate("main", "port"));
    parser.addOption(srcPortOption);
    srcPortOption.setDefaultValue("11111");

    QCommandLineOption dstPortOption(QStringList() << "d" << "dst",
        QCoreApplication::translate("main", "Port for tx packet. Default \"11112\"."),
        QCoreApplication::translate("main", "port"));
    parser.addOption(dstPortOption);
    dstPortOption.setDefaultValue("11112");

    QCommandLineOption srcAddrOption(QStringList() << "src-addr",
        QCoreApplication::translate("main", "Ip address for server. Default \"127.0.0.1\""),
        QCoreApplication::translate("main", "ip"));
    parser.addOption(srcAddrOption);
    srcAddrOption.setDefaultValue("127.0.0.1");

    QCommandLineOption dstAddrOption(QStringList() << "dst-addr",
        QCoreApplication::translate("main", "Ip address for client. Default \"127.0.0.1\""),
        QCoreApplication::translate("main", "ip"));
    parser.addOption(dstAddrOption);
    dstAddrOption.setDefaultValue("127.0.0.1");

    // Process the actual command line arguments given by the user
    parser.process(app);

    // ────────── APPLICATION ──────────────────────────────────────

    // Register types for to use SharedDatagram in signals
    Net::Udp::Utils::registerTypes();

    // Create the app and start it
    App echo;
    bool ok;
    const auto src = parser.value(srcPortOption).toInt(&ok);
    if (ok)
        echo.src = src;
    const auto dst = parser.value(dstPortOption).toInt(&ok);
    if (ok)
        echo.dst = dst;
    const auto srcAddr = parser.value(srcAddrOption);
    if (!srcAddr.isEmpty())
        echo.srcAddr = srcAddr;
    const auto dstAddr = parser.value(dstAddrOption);
    if (!dstAddr.isEmpty())
        echo.dstAddr = dstAddr;
    echo.multiThreaded = parser.isSet(multiThreadOption);

    echo.start();

    // Start event loop
    return QCoreApplication::exec();
}
