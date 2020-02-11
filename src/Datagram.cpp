// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Datagram.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Udp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

void Datagram::reset()
{
    destinationAddress = {};
    destinationPort = 0;
    senderAddress = {};
    senderPort = 0;
    ttl = 8;
}

void Datagram::reset(size_t length)
{
    Datagram::reset();
}
