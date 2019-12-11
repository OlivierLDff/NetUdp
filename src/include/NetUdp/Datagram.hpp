#ifndef __NETUDP_DATAGRAM_HPP__
#define __NETUDP_DATAGRAM_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// C Header

// C++ Header
#include <cstdint>
#include <memory>

// Qt Header
#include <QHostAddress>

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

class NETUDP_API_ Datagram
{
public:
    std::unique_ptr<uint8_t[]> buffer;
    size_t length = 0;

    QHostAddress destinationAddress;
    uint16_t destinationPort = 0;

    QHostAddress senderAddress;
    uint16_t senderPort = 0;

    uint8_t ttl = 8;

    static void registerType();
};

typedef std::shared_ptr<Datagram> SharedDatagram;

NETUDP_NAMESPACE_END

#endif // __NETUDP_DATAGRAM_HPP__
