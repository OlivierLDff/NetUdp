// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Version.hpp>

// Dependencies Header
#include <Stringify/VersionRegex.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Udp;

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
