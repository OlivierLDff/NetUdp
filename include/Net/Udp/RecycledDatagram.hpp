#ifndef __NETUDP_RECYCLED_DATAGRAM_HPP__
#define __NETUDP_RECYCLED_DATAGRAM_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Datagram.hpp>

// Dependencies Headers
#include <Recycler/Buffer.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ RecycledDatagram : public Datagram
{
    // ────── CONSTRUCTOR ────────
public:
    RecycledDatagram(const size_t length);
    void reset() override final;
    void reset(const size_t length) override final;

private:
    Recycler::Buffer<uint8_t> _buffer;

    // ────── API ────────
public:
    uint8_t* buffer() override final;
    const uint8_t* buffer() const override final;
    size_t length() const override final;
};

}
}

#endif
