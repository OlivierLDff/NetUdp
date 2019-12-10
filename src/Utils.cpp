// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header
#include <QCoreApplication> // Call register type at startup when loaded as a dynamic library
#include <QLoggingCategory> // Logging support

// Dependencies Header

// Application Header
#include <NetUdp/Utils.hpp>
#include <NetUdp/Datagram.hpp>
#include <NetUdp/AbstractServer.hpp>
#include <NetUdp/Server.hpp>
#include <NetUdp/Version.hpp>

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

static void registerTypes()
{
    qCDebug(NETUDP_UTILS_LOG_CAT, "Register Singleton %s.Version %d.%d to QML", *_uri, _major, _minor);
    NETUDP_NAMESPACE::Version::registerSingleton(*_uri, _major, _minor);

    qCDebug(NETUDP_UTILS_LOG_CAT, "Register %s.Server %d.%d to QML", *_uri, _major, _minor);
    NETUDP_NAMESPACE::Server::registerToQml(*_uri, _major, _minor);

    qCDebug(NETUDP_UTILS_LOG_CAT, "Register SharedDatagram to QML");
    NETUDP_NAMESPACE::Datagram::registerType();
}

static void registerTypes(const char* uri, const quint8 major, const quint8 minor)
{
    if(uri)
        _uri = &uri;
    _major = major;
    _minor = minor;
    registerTypes();
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

NETUDP_USING_NAMESPACE;

void Utils::registerTypes(const char* uri, const quint8 major, const quint8 minor)
{
    ::registerTypes(uri, major, minor);
}

void Utils::loadResources()
{
    Q_INIT_RESOURCE(NetUdp);
}
