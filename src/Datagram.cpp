// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Datagram.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace net::udp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

void Datagram::reset()
{
    destinationAddress = QString();
    destinationPort = 0;
    senderAddress = QString();
    senderPort = 0;
    ttl = 8;
}

void Datagram::reset(std::size_t length) { Datagram::reset(); }
