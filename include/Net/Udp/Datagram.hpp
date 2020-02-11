#ifndef __NETUDP_DATAGRAM_HPP__
#define __NETUDP_DATAGRAM_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Export.hpp>

// Dependencies Headers

// Qt Header
#include <QHostAddress>

// C++ Header
#include <cstdint>
#include <memory>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ Datagram
{
    // ────── CONSTRUCTOR ────────
public:
    virtual ~Datagram() = default;
    virtual void reset();
    virtual void reset(size_t length);

    // ────── API ────────
public:
    virtual uint8_t* buffer() = 0;
    virtual const uint8_t* buffer() const = 0;
    virtual size_t length() const = 0;

    // ────── ATTRIBUTES ────────
public:
    QHostAddress destinationAddress;
    uint16_t destinationPort = 0;

    QHostAddress senderAddress;
    uint16_t senderPort = 0;

    uint8_t ttl = 8;
};

typedef std::shared_ptr<Datagram> SharedDatagram;

}
}

#endif // __NETUDP_DATAGRAM_HPP__
