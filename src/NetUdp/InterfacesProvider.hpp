// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __NETUDP_INTERFACE_FINDER_HPP__
#define __NETUDP_INTERFACE_FINDER_HPP__

// ──── INCLUDE ────

// Library Headers
#include <NetUdp/Export.hpp>
#include <NetUdp/Property.hpp>

// Dependencies Headers
#include <QString>

// Standard Header
#include <memory>
#include <vector>

// ──── DECLARATION ────

namespace net {
namespace udp {

// ──── CLASS ────

class NETUDP_API_ IInterface
{
public:
    virtual ~IInterface()
    {
    }

    virtual bool isValid() const = 0;

    virtual QString name() const = 0;

    virtual bool isUp() const = 0;
    virtual bool isRunning() const = 0;
    virtual bool canBroadcast() const = 0;
    virtual bool isLoopBack() const = 0;
    virtual bool isPointToPoint() const = 0;
    virtual bool canMulticast() const = 0;
};

using InterfacePtr = std::shared_ptr<const IInterface>;
using InterfacePtrList = std::vector<InterfacePtr>;

class NETUDP_API_ InterfacesProvider
{
    // ──── TYPES ────
public:
    class IProvider
    {
    public:
        virtual ~IProvider() = default;
        virtual InterfacePtrList allInterfaces(bool allowCache = true) const = 0;
        virtual InterfacePtr interfaceFromName(const QString& name, bool allowCache = true) const = 0;
    };
    using ProviderPtr = std::unique_ptr<IProvider>;

    // ──── API ────
public:
    static void setProvider(ProviderPtr p);
    static InterfacePtrList allInterfaces(bool allowCache = true);
    static InterfacePtr interfaceFromName(const QString& name, bool allowCache = true);

private:
    static ProviderPtr _provider;
};

class NETUDP_API_ InterfacesProviderSingleton : public QObject
{
    Q_OBJECT
    NETUDP_SINGLETON_IMPL(InterfacesProviderSingleton, interfacesProvider, InterfacesProvider)

public:
    InterfacesProviderSingleton(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

protected:
    NETUDP_PROPERTY_RO(QStringList, interfaces, Interfaces);

public Q_SLOTS:
    void fetchInterfaces();
};

}
}

#endif
