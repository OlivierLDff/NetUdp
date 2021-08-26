// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
