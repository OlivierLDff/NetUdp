#ifndef __NETUDP_PCH_HPP__
#define __NETUDP_PCH_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Stl
#include <cstdint>
#include <memory>
#include <set>

// spdlog
#include <spdlog/logger.h>
#include <spdlog/sinks/sink.h>

// Recycler
#include <Recycler/Recycler.hpp>

// Qt Core Headers
#include <QtGlobal>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>

// Qt Qml Headers
#include <QQmlEngine>

// Qt Network Headers
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QUdpSocket>

#endif