#ifndef __NETUDP_UTILS_HPP__
#define __NETUDP_UTILS_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C++ Header
#include <QtCore/QtGlobal>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net {
namespace udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

void registerQmlTypes(const char* uri = nullptr, const quint8 major = 1, const quint8 minor = 0);
void loadQmlResources();

}
}

#endif
