// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <NetUdp/NetUdp.hpp>
#include <Qaterial/Qaterial.hpp>

// spdlog
#ifdef WIN32
#    include <spdlog/sinks/msvc_sink.h>
#endif
#ifndef NDEBUG
#    include <spdlog/sinks/stdout_color_sinks.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
#ifdef WIN32
    const auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvcSink->set_level(spdlog::level::debug);
    netudp::Logger::registerSink(msvcSink);
#endif

#ifndef NDEBUG
    const auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdoutSink->set_level(spdlog::level::debug);
    netudp::Logger::registerSink(stdoutSink);
#endif

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    engine.addImportPath("qrc:///");

    netudp::loadQmlResources();
    netudp::registerQmlTypes();

    qaterial::loadQmlResources();
    qaterial::registerQmlTypes();

    Q_INIT_RESOURCE(EchoQml);

    engine.load(QUrl("qrc:/EchoQml.qml"));
    if(engine.rootObjects().isEmpty())
        return -1;

    return QGuiApplication::exec();
}
