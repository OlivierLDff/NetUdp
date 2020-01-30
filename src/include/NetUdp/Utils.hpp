#ifndef __NETUDP_UTILS_HPP__
#define __NETUDP_UTILS_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <NetUdp/Export.hpp>

// C++ Header
#include <QtGlobal>

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
    static void registerTypes(const char* uri = nullptr, const quint8 major = 1, const quint8 minor = 0);
    static void loadResources();
};

NETUDP_NAMESPACE_END // __NETUDP_UTILS_HPP__

#endif
