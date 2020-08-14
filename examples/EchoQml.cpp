// ──── INCLUDE ────

#include <Net/Udp/NetUdp.hpp>

// spdlog
#ifdef WIN32
#    include <spdlog/sinks/msvc_sink.h>
#endif
#ifndef NDEBUG
#    include <spdlog/sinks/stdout_color_sinks.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>

// ──── FUNCTIONS ────

int main(int argc, char* argv[])
{
#ifdef WIN32
    const auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvcSink->set_level(spdlog::level::debug);
    net::udp::Logger::registerSink(msvcSink);
#endif

#ifndef NDEBUG
    const auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdoutSink->set_level(spdlog::level::debug);
    net::udp::Logger::registerSink(stdoutSink);
#endif

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    engine.addImportPath("qrc:///");

    net::udp::loadQmlResources();
    net::udp::registerQmlTypes();

    Q_INIT_RESOURCE(EchoQml);

    engine.load(QUrl("qrc:/EchoQml.qml"));
    if(engine.rootObjects().isEmpty())
        return -1;

    return QGuiApplication::exec();
}
