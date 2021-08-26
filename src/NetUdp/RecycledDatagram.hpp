// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __NETUDP_RECYCLED_DATAGRAM_HPP__
#define __NETUDP_RECYCLED_DATAGRAM_HPP__

#include <NetUdp/Datagram.hpp>
#include <Recycler/Buffer.hpp>

namespace netudp {

class NETUDP_API_ RecycledDatagram : public Datagram
{
    // ────── CONSTRUCTOR ────────
public:
    RecycledDatagram(const std::size_t length);
    void reset() override final;
    void reset(const std::size_t length) override final;
    void resize(std::size_t length) override;

private:
    recycler::Buffer<std::uint8_t> _buffer;

    // ────── API ────────
public:
    std::uint8_t* buffer() override final;
    const std::uint8_t* buffer() const override final;
    std::size_t length() const override final;
};

}

#endif
