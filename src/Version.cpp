// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Version.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net::udp {

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

Version::Version(QObject* parent) :
    QObject(parent), _major(NETUDP_VERSION_MAJOR), _minor(NETUDP_VERSION_MINOR), _patch(NETUDP_VERSION_PATCH),
    _tag(NETUDP_VERSION_TAG_HEX),
    _readable(QString::number(_major) + QStringLiteral(".") + QString::number(_minor) + QStringLiteral(".") +
              QString::number(_patch) + QStringLiteral(".0x") + QString::number(_tag, 16).rightJustified(8, QChar('0')))
{
}

}
