#ifndef __NETUDP_UTILS_HPP__
#define __NETUDP_UTILS_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header

// Qt Header

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
class NETUDP_API_ Utils
{
public:
    /**
     * Register type to the qml engines
     * Registered types are:
     * -
     */
    static void registerTypes(const char* uri, const quint8 major, const quint8 minor);
};

NETUDP_NAMESPACE_END // __NETUDP_UTILS_HPP__

#endif
