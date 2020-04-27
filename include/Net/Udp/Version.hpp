#ifndef __NETUDP_COMMON_HPP__
#define __NETUDP_COMMON_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Export.hpp>
#include <Net/Udp/Property.hpp>

// Qt Header
#include <QObject>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net {
namespace udp {

class NETUDP_API_ Version : public QObject
{
    Q_OBJECT
    NETUDP_SINGLETON_IMPL(Version, version, Version);

    // ──────── CONSTRUCTOR ────────────────
public:
    Version(QObject* parent = nullptr);

    // ──────── ATTRIBUTES ────────────────
private:
    /** \brief Library Major Version */
    NETUDP_PROPERTY_CONST(quint32, major, Major);

    /** \brief Library Minor Version */
    NETUDP_PROPERTY_CONST(quint32, minor, Minor);

    /** \brief Library Patch Version */
    NETUDP_PROPERTY_CONST(quint32, patch, Patch);

    /** \brief Library Tag Version */
    NETUDP_PROPERTY_CONST(quint32, tag, Tag);

    /** \brief Library Version as major.minor.patch.tag */
    NETUDP_PROPERTY_CONST(QString, readable, Readable);
};

}
}

#endif
