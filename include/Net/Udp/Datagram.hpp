#ifndef __NETUDP_DATAGRAM_HPP__
#define __NETUDP_DATAGRAM_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Export.hpp>

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

    // todo : inherit from Recycled::Buffer
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

    static std::shared_ptr<Datagram> makeDatagram() { return std::make_shared<Datagram>(); }

    bool setBufferSize(size_t length)
    {
        if(length <= 0)
        {
            this->length = 0;
            buffer = nullptr;
            return false;
        }

        this->length = length;
        buffer = std::make_unique<uint8_t[]>(length);
        return true;
    }
};

typedef std::shared_ptr<Datagram> SharedDatagram;

}
}

#endif // __NETUDP_DATAGRAM_HPP__
