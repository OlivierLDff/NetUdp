#ifndef __NETUDP_TEMPLATE_HPP__
#define __NETUDP_TEMPLATE_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header
#include <QObject>

// Dependencies Header

// Application Header
#include <NetUdp/Export.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

NETUDP_NAMESPACE_START

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

/**
 */
class NETUDP_API_ ITemplate
{
public:
    virtual ~Template() = default;
};

/**
 */
class NETUDP_API_ BaseTemplate: public QObject, public ITemplate
{
    Q_OBJECT

public:
    BaseTemplate(QObject* parent = nullptr);
    virtual ~BaseTemplate() = default;
};

/**
 */
class NETUDP_API_ Template: public BaseTemplate
{
    Q_OBJECT

public:
    Template(QObject* parent = nullptr);
    virtual ~Template() = default;
};


NETUDP_NAMESPACE_END

#endif // __NETUDP_TEMPLATE_HPP__
