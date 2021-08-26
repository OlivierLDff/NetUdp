// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <NetUdp/RecycledDatagram.hpp>
#include <Recycler/Buffer.hpp>

namespace netudp {

struct RecycledDatagramPrivate
{
    RecycledDatagramPrivate(const std::size_t length)
        : buffer(length)
    {
    }
    recycler::Buffer<std::uint8_t> buffer;
};

RecycledDatagram::RecycledDatagram(const std::size_t length)
    : _p(std::make_unique<RecycledDatagramPrivate>(length))
{
}

RecycledDatagram::~RecycledDatagram() = default;

void RecycledDatagram::reset()
{
    _p->buffer.reset(0);
    Datagram::reset();
}

void RecycledDatagram::reset(const std::size_t length)
{
    _p->buffer.reset(length);
    Datagram::reset(length);
}

void RecycledDatagram::resize(std::size_t length)
{
    _p->buffer.resize(length);
}

std::uint8_t* RecycledDatagram::buffer()
{
    return _p->buffer;
}

const std::uint8_t* RecycledDatagram::buffer() const
{
    return _p->buffer;
}

std::size_t RecycledDatagram::length() const
{
    return _p->buffer.length();
}

}
