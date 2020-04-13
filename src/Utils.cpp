// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Utils.hpp>
#include <Net/Udp/RecycledDatagram.hpp>
#include <Net/Udp/AbstractServer.hpp>
#include <Net/Udp/Server.hpp>
#include <Net/Udp/Version.hpp>
#include <Net/Udp/Logger.hpp>

// Qt Header
#include <QCoreApplication>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
# define LOG_DEV_DEBUG(str, ...) Net::Udp::Logger::UTILS->debug(str, ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  do {} while (0)
#else
# define LOG_DEV_INFO(str, ...)  Net::Udp::Logger::UTILS->info( str, ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  do {} while (0)
#else
# define LOG_DEV_WARN(str, ...)  Net::Udp::Logger::UTILS->warn( str, ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   do {} while (0)
#else
# define LOG_DEV_ERR(str, ...)   Net::Udp::Logger::UTILS->error(str, ## __VA_ARGS__);
#endif

#define LOG_DEBUG(str, ...)      Net::Udp::Logger::UTILS->debug(str, ## __VA_ARGS__);
#define LOG_INFO(str, ...)       Net::Udp::Logger::UTILS->info( str, ## __VA_ARGS__);
#define LOG_WARN(str, ...)       Net::Udp::Logger::UTILS->warn( str, ## __VA_ARGS__);
#define LOG_ERR(str, ...)        Net::Udp::Logger::UTILS->error(str, ## __VA_ARGS__);


// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

static const char* _defaultUri = "NetUdp";
static const char** _uri = &_defaultUri;
static quint8 _major = 1;
static quint8 _minor = 0;

static void NetUdp_registerTypes()
{
    LOG_DEV_INFO("Register NetUdp v{}", qPrintable(Net::Udp::Version::version().readable()));

    LOG_DEV_INFO("Register Singleton {}.Version {}.{} to QML", *_uri, _major, _minor);
    Net::Udp::Version::registerSingleton(*_uri, _major, _minor);

    LOG_DEV_INFO("Register {}.AbstractServer {}.{} to QML", *_uri, _major, _minor);
    Net::Udp::AbstractServer::registerToQml(*_uri, _major, _minor);

    LOG_DEV_INFO("Register {}.Server {}.{} to QML", *_uri, _major, _minor);
    Net::Udp::Server::registerToQml(*_uri, _major, _minor);

    qRegisterMetaType<QAbstractSocket::SocketState>();

    LOG_DEV_INFO("Register Net::Udp::SharedDatagram to QML");
    qRegisterMetaType<Net::Udp::SharedDatagram>("Net::Udp::SharedDatagram");
    qRegisterMetaType<Net::Udp::SharedDatagram>("Udp::SharedDatagram");
    qRegisterMetaType<Net::Udp::SharedDatagram>("SharedDatagram");
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
    LOG_DEV_INFO("Load NetUdp.qrc v{}", qPrintable(Net::Udp::Version::version().readable()));
    Q_INIT_RESOURCE(NetUdp);
}

#ifndef NETUDP_STATIC
Q_COREAPP_STARTUP_FUNCTION(NetUdp_registerTypes)
Q_COREAPP_STARTUP_FUNCTION(NetUdp_loadResources)
#endif

using namespace Net::Udp;

void Utils::registerTypes(const char* uri, const quint8 major, const quint8 minor)
{
    ::NetUdp_registerTypes(uri, major, minor);
}

void Utils::loadResources()
{
    ::NetUdp_loadResources();
}
