#ifndef __NETUDP_PCH_HPP__
#define __NETUDP_PCH_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Stl
#include <cstdint>
#include <memory>
#include <set>

// Qt Core Headers
#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QElapsedTimer>

// Compatibility with gcc <= 7
#ifdef major
#    undef major
#endif
#ifdef minor
#    undef minor
#endif

#endif
