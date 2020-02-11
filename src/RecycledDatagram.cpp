// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/RecycledDatagram.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Udp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

RecycledDatagram::RecycledDatagram(const size_t length): _buffer(length)
{
}

void RecycledDatagram::reset()
{
    _buffer.reset(0);
    Datagram::reset();
}

void RecycledDatagram::reset(const size_t length)
{
    _buffer.reset(length);
    Datagram::reset(length);
}

uint8_t* RecycledDatagram::buffer()
{
    return _buffer;
}

const uint8_t* RecycledDatagram::buffer() const
{
    return _buffer;
}

size_t RecycledDatagram::length() const
{
    return _buffer.length();;
}
