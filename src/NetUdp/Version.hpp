// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __NETUDP_COMMON_HPP__
#define __NETUDP_COMMON_HPP__

#include <NetUdp/Export.hpp>
#include <NetUdp/Property.hpp>
#include <QtCore/QObject>

namespace netudp {

class NETUDP_API_ Version : public QObject
{
    Q_OBJECT
    NETUDP_SINGLETON_IMPL(Version, version, Version);

    // ──────── CONSTRUCTOR ────────────────
public:
    Version(QObject* parent = nullptr);

    // ──────── ATTRIBUTES ────────────────
private:
    /** \brief Library Major Version */
    NETUDP_PROPERTY_CONST(quint32, major, Major);

    /** \brief Library Minor Version */
    NETUDP_PROPERTY_CONST(quint32, minor, Minor);

    /** \brief Library Patch Version */
    NETUDP_PROPERTY_CONST(quint32, patch, Patch);

    /** \brief Library Tag Version */
    NETUDP_PROPERTY_CONST(quint32, tag, Tag);

    /** \brief Library Version as major.minor.patch.tag */
    NETUDP_PROPERTY_CONST(QString, readable, Readable);
};

}

#endif
