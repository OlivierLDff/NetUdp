// Copyright 2019 - 2021 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <NetUdp/InterfacesProvider.hpp>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAddressEntry>
#include <chrono>
#include <mutex>

namespace netudp {

class QRealNetworkIface : public IInterface
{
public:
    QRealNetworkIface(const QNetworkInterface& iface)
        : _iface(iface)
    {
    }

    bool isValid() const override
    {
        return _iface.isValid();
    }
    QString name() const override
    {
        return _iface.name();
    }

    bool isUp() const override
    {
        return _iface.flags() & QNetworkInterface::IsUp;
    }
    bool isRunning() const override
    {
        return _iface.flags() & QNetworkInterface::IsRunning;
    }
    bool canBroadcast() const override
    {
        return _iface.flags() & QNetworkInterface::CanBroadcast;
    }
    bool isLoopBack() const override
    {
        return _iface.flags() & QNetworkInterface::IsLoopBack;
    }
    bool isPointToPoint() const override
    {
        return _iface.flags() & QNetworkInterface::IsPointToPoint;
    }
    bool canMulticast() const override
    {
        return _iface.flags() & QNetworkInterface::CanMulticast;
    }

private:
    QNetworkInterface _iface;

public:
    static InterfacePtr make(const QNetworkInterface& iface)
    {
        return std::make_shared<QRealNetworkIface>(iface);
    }
};

class QRealNetworkIFaceProvider : public InterfacesProvider::IProvider
{
public:
    static std::uint64_t now()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    mutable std::uint64_t lastCacheFetch = 0;
    mutable InterfacePtrList cache;
    mutable bool fetchedOnce = false;
    mutable std::mutex mutex;

    [[nodiscard]] InterfacePtrList allInterfaces(bool allowCache) const override
    {
        mutex.lock();
        const auto ms = now();
        if(allowCache && fetchedOnce && (ms - lastCacheFetch) < 3000)
        {
            mutex.unlock();
            return cache;
        }

        InterfacePtrList result;
        for(const auto& iface: QNetworkInterface::allInterfaces())
        {
            result.push_back(QRealNetworkIface::make(iface));
        }
        cache = result;
        fetchedOnce = true;
        lastCacheFetch = ms;

        mutex.unlock();
        return result;
    }
    [[nodiscard]] InterfacePtr interfaceFromName(const QString& name, bool allowCache) const override
    {
        mutex.lock();
        if(allowCache && fetchedOnce && (now() - lastCacheFetch) < 3000)
        {
            for(const auto& iface: cache)
            {
                if(iface->name() == name)
                {
                    mutex.unlock();
                    return iface;
                }
            }
        }

        mutex.unlock();
        return QRealNetworkIface::make(QNetworkInterface::interfaceFromName(name));
    }
};

InterfacesProvider::ProviderPtr InterfacesProvider::_provider = std::make_unique<QRealNetworkIFaceProvider>();

// ──── FUNCTIONS ────

void InterfacesProvider::setProvider(ProviderPtr p)
{
    _provider = p ? std::move(p) : std::make_unique<QRealNetworkIFaceProvider>();
}

InterfacePtrList InterfacesProvider::allInterfaces(bool allowCache)
{
    Q_ASSERT(_provider);
    return _provider->allInterfaces(allowCache);
}

InterfacePtr InterfacesProvider::interfaceFromName(const QString& name, bool allowCache)
{
    Q_ASSERT(InterfacesProvider::_provider);
    return _provider->interfaceFromName(name, allowCache);
}

void InterfacesProviderSingleton::fetchInterfaces()
{
    QStringList result;
    for(const auto& interface: InterfacesProvider::allInterfaces())
    {
        if(interface->isValid() && interface->isRunning() && interface->isUp() && interface->canMulticast())
            result.push_back(interface->name());
    }
    setInterfaces(result);
}

}
