// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Utils.hpp>
#include <Net/Udp/RecycledDatagram.hpp>
#include <Net/Udp/AbstractServer.hpp>
#include <Net/Udp/Server.hpp>
#include <Net/Udp/Version.hpp>

// Qt Header
#include <QCoreApplication>
#include <QLoggingCategory>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(NETUDP_UTILS_LOG_CAT, "net.udp.utils")

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

static const char* _defaultUri = "NetUdp";
static const char** _uri = &_defaultUri;
static quint8 _major = 1;
static quint8 _minor = 0;

static void NetUdp_registerTypes()
{
    qCDebug(NETUDP_UTILS_LOG_CAT, "Register NetUdp v%s", qPrintable(Net::Udp::Version::version().readable()));

    qCDebug(NETUDP_UTILS_LOG_CAT, "Register Singleton %s.Version %d.%d to QML", *_uri, _major, _minor);
    Net::Udp::Version::registerSingleton(*_uri, _major, _minor);

    qCDebug(NETUDP_UTILS_LOG_CAT, "Register %s.AbstractServer %d.%d to QML", *_uri, _major, _minor);
    Net::Udp::AbstractServer::registerToQml(*_uri, _major, _minor);

    qCDebug(NETUDP_UTILS_LOG_CAT, "Register %s.Server %d.%d to QML", *_uri, _major, _minor);
    Net::Udp::Server::registerToQml(*_uri, _major, _minor);

    qRegisterMetaType<QAbstractSocket::SocketState>();

    qCDebug(NETUDP_UTILS_LOG_CAT, "Register Net::Udp::SharedDatagram to QML");
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
    qCDebug(NETUDP_UTILS_LOG_CAT, "Load NetUdp.qrc v%s", qPrintable(Net::Udp::Version::version().readable()));
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
