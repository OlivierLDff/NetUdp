// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header
#include <QString>

// Dependencies Header

// Application Header
#include <NetUdp/Version.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

NETUDP_USING_NAMESPACE

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

Version::Version(QObject* parent): QObject(parent),
    _major(NETUDP_VERSION_MAJOR),
    _minor(NETUDP_VERSION_MINOR),
    _patch(NETUDP_VERSION_PATCH),
    _tag(NETUDP_VERSION_TAG_HEX),
    _readable(QString::number(_major) + "." +
        QString::number(_minor) + "." +
        QString::number(_patch) + "." +
        QString::number(_tag, 16))
{
}
