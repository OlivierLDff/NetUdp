#ifndef __NETUDP_COMMON_HPP__
#define __NETUDP_COMMON_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <NetUdp/Export.hpp>

// Qt Header
#include <QObject>
#include <QQmlSingletonHelper.h>
#include <QQmlAutoPropertyHelpers.h>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

NETUDP_NAMESPACE_START

class NETUDP_API_ Version : public QObject
{
    Q_OBJECT
    QSM_SINGLETON_IMPL(Version, version, Version);

    // ──────── CONSTRUCTOR ────────────────
public:
    Version(QObject* parent = nullptr);

    // ──────── ATTRIBUTES ────────────────
private:
    /** \brief Library Major Version */
    QSM_CONSTANT_AUTO_PROPERTY(quint32, major, Major)

    /** \brief Library Minor Version */
    QSM_CONSTANT_AUTO_PROPERTY(quint32, minor, Minor)

    /** \brief Library Patch Version */
    QSM_CONSTANT_AUTO_PROPERTY(quint32, patch, Patch)

    /** \brief Library Tag Version */
    QSM_CONSTANT_AUTO_PROPERTY(quint32, tag, Tag)

    /** \brief Library Version as major.minor.patch.tag */
    QSM_CONSTANT_AUTO_PROPERTY(QString, readable, Readable)
};

NETUDP_NAMESPACE_END

#endif
