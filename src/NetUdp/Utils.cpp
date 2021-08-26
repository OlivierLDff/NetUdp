// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <NetUdp/Utils.hpp>
#include <NetUdp/RecycledDatagram.hpp>
#include <NetUdp/Socket.hpp>
#include <NetUdp/Version.hpp>
#include <NetUdp/Logger.hpp>
#include <NetUdp/InterfacesProvider.hpp>

// Qt Header
#include <QtCore/QCoreApplication>
#include <QtNetwork/QAbstractSocket>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

// clang-format off
#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
# define LOG_DEV_DEBUG(str, ...) net::udp::Logger::UTILS->debug(str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  do {} while (0)
#else
# define LOG_DEV_INFO(str, ...)  net::udp::Logger::UTILS->info( str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  do {} while (0)
#else
# define LOG_DEV_WARN(str, ...)  net::udp::Logger::UTILS->warn( str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   do {} while (0)
#else
# define LOG_DEV_ERR(str, ...)   net::udp::Logger::UTILS->error(str, ## __VA_ARGS__)
#endif

#define LOG_DEBUG(str, ...)      net::udp::Logger::UTILS->debug(str, ## __VA_ARGS__)
#define LOG_INFO(str, ...)       net::udp::Logger::UTILS->info( str, ## __VA_ARGS__)
#define LOG_WARN(str, ...)       net::udp::Logger::UTILS->warn( str, ## __VA_ARGS__)
#define LOG_ERR(str, ...)        net::udp::Logger::UTILS->error(str, ## __VA_ARGS__)
// clang-format on

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

static const char* _defaultUri = "NetUdp";
static const char** _uri = &_defaultUri;
static quint8 _major = 1;
static quint8 _minor = 0;

static void NetUdp_registerTypes()
{
    LOG_DEV_INFO("Register NetUdp v{}", qPrintable(net::udp::Version::version().readable()));

    LOG_DEV_INFO("Register Singleton {}.Version {}.{} to QML", *_uri, _major, _minor);
    net::udp::Version::registerSingleton(*_uri, _major, _minor);

    LOG_DEV_INFO("Register {}.Server {}.{} to QML", *_uri, _major, _minor);
    net::udp::Socket::registerToQml(*_uri, _major, _minor);

    LOG_DEV_INFO("Register Singleton {}.InterfacesProvider {}.{} to QML", *_uri, _major, _minor);
    net::udp::InterfacesProviderSingleton::registerSingleton(*_uri, _major, _minor);

    qRegisterMetaType<QAbstractSocket::SocketState>();

    LOG_DEV_INFO("Register net::udp::SharedDatagram to QML");
    qRegisterMetaType<net::udp::SharedDatagram>("net::udp::SharedDatagram");
    qRegisterMetaType<net::udp::SharedDatagram>("udp::SharedDatagram");
    qRegisterMetaType<net::udp::SharedDatagram>("SharedDatagram");
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
    LOG_DEV_INFO("Load NetUdp.qrc v{}", qPrintable(net::udp::Version::version().readable()));
    Q_INIT_RESOURCE(NetUdp);
#endif
}

#ifndef NETUDP_STATIC
Q_COREAPP_STARTUP_FUNCTION(NetUdp_registerTypes)
Q_COREAPP_STARTUP_FUNCTION(NetUdp_loadResources)
#endif

using namespace net::udp;

void net::udp::registerQmlTypes(const char* uri, const quint8 major, const quint8 minor)
{
    ::NetUdp_registerTypes(uri, major, minor);
}

void net::udp::loadQmlResources()
{
    ::NetUdp_loadResources();
}
