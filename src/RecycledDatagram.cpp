// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/RecycledDatagram.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace net::udp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

RecycledDatagram::RecycledDatagram(const std::size_t length) : _buffer(length) {}

void RecycledDatagram::reset()
{
    _buffer.reset(0);
    Datagram::reset();
}

void RecycledDatagram::reset(const std::size_t length)
{
    _buffer.reset(length);
    Datagram::reset(length);
}

std::uint8_t* RecycledDatagram::buffer() { return _buffer; }

const std::uint8_t* RecycledDatagram::buffer() const { return _buffer; }

std::size_t RecycledDatagram::length() const {
    return _buffer.length();
}
