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

namespace net {
namespace udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ RecycledDatagram : public Datagram
{
    // ────── CONSTRUCTOR ────────
public:
    RecycledDatagram(const std::size_t length);
    void reset() override final;
    void reset(const std::size_t length) override final;

private:
    recycler::Buffer<std::uint8_t> _buffer;

    // ────── API ────────
public:
    std::uint8_t* buffer() override final;
    const std::uint8_t* buffer() const override final;
    std::size_t length() const override final;
};

}
}

#endif
