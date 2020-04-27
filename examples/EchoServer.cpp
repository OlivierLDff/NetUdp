
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
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QTimer>
#include <QNetworkDatagram>
#include <QUdpSocket>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(APP_LOG_CAT, "app")
Q_LOGGING_CATEGORY(SERVER_LOG_CAT, "server")

class App
{
public:
    int counter = 0;

    uint16_t src = 11111;
    uint16_t dst = 11112;

    QString srcAddr = QStringLiteral("127.0.0.1");
    QString dstAddr = QStringLiteral("127.0.0.1");

    net::udp::Socket server;

    bool multiThreaded = false;

    QTimer timer;

public:
    void start()
    {
        qCInfo(APP_LOG_CAT, "Init application");

        qCInfo(SERVER_LOG_CAT, "Set Rx Address to %s:%d", qPrintable(srcAddr), signed(src));

        server.setRxAddress(srcAddr);
        server.setRxPort(src);
        server.setUseWorkerThread(multiThreaded);

        QObject::connect(&timer, &QTimer::timeout,
            [this]()
            {
                const std::string data = "Echo " + std::to_string(counter++);
                server.sendDatagram(data.c_str(), data.length() + 1, dstAddr, dst);
            });

        QObject::connect(&server, &net::udp::Socket::datagramReceived,
            [](const net::udp::SharedDatagram& d)
            { qCInfo(SERVER_LOG_CAT, "Rx : %s", reinterpret_cast<const char*>(d->buffer())); });

        qCInfo(APP_LOG_CAT, "Start application");

        QObject::connect(&server, &net::udp::Socket::isRunningChanged,
            [](bool value) { qCInfo(SERVER_LOG_CAT, "isRunning : %d", signed(value)); });
        QObject::connect(&server, &net::udp::Socket::isBoundedChanged,
            [](bool value) { qCInfo(SERVER_LOG_CAT, "isBounded : %d", signed(value)); });
        QObject::connect(&server, &net::udp::Socket::socketError,
            [](int value, const QString error)
            { qCInfo(SERVER_LOG_CAT, "error : %s", qPrintable(error)); });

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

    QCommandLineOption multiThreadOption(
        QStringList() << "t", QCoreApplication::translate("main",
                                  "Make the worker live in a different thread. Default false"));
    parser.addOption(multiThreadOption);

    QCommandLineOption srcPortOption(QStringList() << "s"
                                                   << "src",
        QCoreApplication::translate("main", "Port for rx packet. Default \"11111\"."),
        QCoreApplication::translate("main", "port"));
    parser.addOption(srcPortOption);
    srcPortOption.setDefaultValue("11111");

    QCommandLineOption dstPortOption(QStringList() << "d"
                                                   << "dst",
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
    net::udp::Utils::registerTypes();

    // Create the app and start it
    App echo;
    bool ok;
    const auto src = parser.value(srcPortOption).toInt(&ok);
    if(ok)
        echo.src = src;
    const auto dst = parser.value(dstPortOption).toInt(&ok);
    if(ok)
        echo.dst = dst;
    const auto srcAddr = parser.value(srcAddrOption);
    if(!srcAddr.isEmpty())
        echo.srcAddr = srcAddr;
    const auto dstAddr = parser.value(dstAddrOption);
    if(!dstAddr.isEmpty())
        echo.dstAddr = dstAddr;
    echo.multiThreaded = parser.isSet(multiThreadOption);

    echo.start();

    // Same example using raw qt

    /*QUdpSocket socket;
    const bool success = socket.bind(QHostAddress("127.0.0.1"), 11111,
        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    if(!success)
        return 0;
    QTimer t;
    QObject::connect(&t, &QTimer::timeout,
        [&]
        {
            socket.writeDatagram(
                QNetworkDatagram(QByteArray("Test", 5), QHostAddress("127.0.0.1"), 11112));
        });
    QObject::connect(&socket, &QUdpSocket::readyRead,
        [&]()
        {
            while(socket.pendingDatagramSize() >= 0)
            {
                QByteArray d = socket.read(1);
                qDebug("available %lld", socket.bytesAvailable());
                qDebug("pendingDatagramSize %lld", socket.pendingDatagramSize());
                qDebug("e %s", qPrintable(socket.errorString()));
                socket.receiveDatagram();
            }
        });
    t.start(1000);*/

    // Start event loop
    return QCoreApplication::exec();
}
