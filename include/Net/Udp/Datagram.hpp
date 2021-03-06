#ifndef __NETUDP_DATAGRAM_HPP__
#define __NETUDP_DATAGRAM_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/Export.hpp>

// Qt Header
#include <QtCore/QString>
#include <QtCore/QMetaType>

// C++ Header
#include <memory>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net {
namespace udp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETUDP_API_ Datagram
{
    // ────── CONSTRUCTOR ────────
public:
    virtual ~Datagram() = default;
    virtual void reset();

    // Reset the datagram and clear the content.
    // Also force the buffer to be initialized with size length
    virtual void reset(std::size_t length);

    // Resize the datagram without destroying the data
    virtual void resize(std::size_t length);

    // ────── API ────────
public:
    virtual std::uint8_t* buffer() = 0;
    virtual const std::uint8_t* buffer() const = 0;
    virtual std::size_t length() const = 0;

    // ────── ATTRIBUTES ────────
public:
    QString destinationAddress;
    quint16 destinationPort = 0;

    QString senderAddress;
    quint16 senderPort = 0;

    quint8 ttl = 0;
};

typedef std::shared_ptr<Datagram> SharedDatagram;

}
}

Q_DECLARE_METATYPE(net::udp::SharedDatagram);

#endif
