// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <NetUdp/Version.hpp>

namespace netudp {

Version::Version(QObject* parent)
    : QObject(parent)
    , _major(NETUDP_VERSION_MAJOR)
    , _minor(NETUDP_VERSION_MINOR)
    , _patch(NETUDP_VERSION_PATCH)
    , _tag(NETUDP_VERSION_TAG_HEX)
    , _readable(QString::number(_major) + QStringLiteral(".") + QString::number(_minor) + QStringLiteral(".") + QString::number(_patch)
                + QStringLiteral(".0x") + QString::number(_tag, 16).rightJustified(8, QChar('0')))
{
}

}
