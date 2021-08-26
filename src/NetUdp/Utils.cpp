// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <NetUdp/Utils.hpp>
#include <NetUdp/RecycledDatagram.hpp>
#include <NetUdp/Socket.hpp>
#include <NetUdp/Version.hpp>
#include <NetUdp/Logger.hpp>
#include <NetUdp/InterfacesProvider.hpp>
#include <QtCore/QCoreApplication>
#include <QtNetwork/QAbstractSocket>

// clang-format off
#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
# define LOG_DEV_DEBUG(str, ...) netudp::Logger::UTILS->debug(str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  do {} while (0)
#else
# define LOG_DEV_INFO(str, ...)  netudp::Logger::UTILS->info( str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  do {} while (0)
#else
# define LOG_DEV_WARN(str, ...)  netudp::Logger::UTILS->warn( str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   do {} while (0)
#else
# define LOG_DEV_ERR(str, ...)   netudp::Logger::UTILS->error(str, ## __VA_ARGS__)
#endif

#define LOG_DEBUG(str, ...)      netudp::Logger::UTILS->debug(str, ## __VA_ARGS__)
#define LOG_INFO(str, ...)       netudp::Logger::UTILS->info( str, ## __VA_ARGS__)
#define LOG_WARN(str, ...)       netudp::Logger::UTILS->warn( str, ## __VA_ARGS__)
#define LOG_ERR(str, ...)        netudp::Logger::UTILS->error(str, ## __VA_ARGS__)
// clang-format on

static const char* _defaultUri = "NetUdp";
static const char** _uri = &_defaultUri;
static quint8 _major = 1;
static quint8 _minor = 0;

static void NetUdp_registerTypes()
{
    LOG_DEV_INFO("Register NetUdp v{}", qPrintable(netudp::Version::version().readable()));

    LOG_DEV_INFO("Register Singleton {}.Version {}.{} to QML", *_uri, _major, _minor);
    netudp::Version::registerSingleton(*_uri, _major, _minor);

    LOG_DEV_INFO("Register {}.Server {}.{} to QML", *_uri, _major, _minor);
    netudp::Socket::registerToQml(*_uri, _major, _minor);

    LOG_DEV_INFO("Register Singleton {}.InterfacesProvider {}.{} to QML", *_uri, _major, _minor);
    netudp::InterfacesProviderSingleton::registerSingleton(*_uri, _major, _minor);

    qRegisterMetaType<QAbstractSocket::SocketState>();

    LOG_DEV_INFO("Register netudp::SharedDatagram to QML");
    qRegisterMetaType<netudp::SharedDatagram>("netudp::SharedDatagram");
    qRegisterMetaType<netudp::SharedDatagram>("udp::SharedDatagram");
    qRegisterMetaType<netudp::SharedDatagram>("SharedDatagram");
}

static void NetUdp_registerTypes(const char* uri, const quint8 major, const quint8 minor)
{
    if(uri)
        _uri = &uri;
    _major = major;
    _minor = minor;
    NetUdp_registerTypes();
}

void NetUdp_loadResources()
{
#ifdef NETUDP_ENABLE_QML
    LOG_DEV_INFO("Load NetUdp.qrc v{}", qPrintable(netudp::Version::version().readable()));
    Q_INIT_RESOURCE(NetUdp);
#endif
}

#ifndef NETUDP_STATIC
Q_COREAPP_STARTUP_FUNCTION(NetUdp_registerTypes)
Q_COREAPP_STARTUP_FUNCTION(NetUdp_loadResources)
#endif

using namespace netudp;

void netudp::registerQmlTypes(const char* uri, const quint8 major, const quint8 minor)
{
    ::NetUdp_registerTypes(uri, major, minor);
}

void netudp::loadQmlResources()
{
    ::NetUdp_loadResources();
}
