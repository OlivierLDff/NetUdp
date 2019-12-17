// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header
#include <QString>

// Dependencies Header
#include <Stringify/VersionRegex.hpp>

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
    _readable(Stringify::VersionRegex::fullVersionToString(_major, _minor, _patch, _tag))
{
}
