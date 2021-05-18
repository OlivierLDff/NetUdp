#ifndef __NETUDP_INTERFACE_FINDER_HPP__
#define __NETUDP_INTERFACE_FINDER_HPP__

// ──── INCLUDE ────

// Library Headers
#include <Net/Udp/Export.hpp>
#include <Net/Udp/Property.hpp>

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
