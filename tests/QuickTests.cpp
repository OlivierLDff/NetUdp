#include <Net/Udp/NetUdp.hpp>

// spdlog
#ifdef WIN32
#    include <spdlog/sinks/msvc_sink.h>
#endif
#ifndef NDEBUG
#    include <spdlog/sinks/stdout_color_sinks.h>
#endif

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtQuickTest/QtQuickTest>

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup()
    {
    }

public slots:
    void qmlEngineAvailable(QQmlEngine* engine)
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

        net::udp::registerQmlTypes();
    }
};

QUICK_TEST_MAIN_WITH_SETUP(QuickTests, Setup)

#include "QuickTests.moc"
