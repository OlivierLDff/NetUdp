// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Udp/AbstractServer.hpp>

// Qt Header
#include <QNetworkInterface>
#include <QHostAddress>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Udp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────
//
AbstractServer::AbstractServer(QObject* parent): QObject(parent)
{
}

const std::set<QString>& AbstractServer::multicastGroupsSet() const
{
    return _multicastGroups;
}

QList<QString> AbstractServer::multicastGroups() const
{
    QList<QString> res;
    for (const auto& it : _multicastGroups)
        res.append(it);
    return res;
}

QString AbstractServer::multicastInterfaceName() const
{
    return _multicastInterfaceName;
}

bool AbstractServer::setMulticastInterfaceName(const QString& name)
{
    if (name != _multicastInterfaceName &&
        (name.isEmpty() || QNetworkInterface::interfaceFromName(name).isValid()))
    {
        _multicastInterfaceName = name;
        Q_EMIT multicastInterfaceNameChanged(_multicastInterfaceName);
        return true;
    }
    return false;
}

bool AbstractServer::start()
{
    // ) We can't start if we are already running
    if (isRunning())
        return false;

    setIsRunning(true);

    return true;
}

bool AbstractServer::stop()
{
    // ) We can't stop if we are not running
    if (!isRunning())
        return false;

    setIsBounded(false);
    setIsRunning(false);

    resetRxBytesPerSeconds();
    resetTxBytesPerSeconds();
    resetRxPacketsPerSeconds();
    resetTxPacketsPerSeconds();

    return true;
}

bool AbstractServer::restart()
{
    stop();
    return start();
}

bool AbstractServer::joinMulticastGroup(const QString& groupAddress)
{
    // ) Check that the address isn't already registered
    if (_multicastGroups.find(groupAddress) != _multicastGroups.end())
        return false;

    // ) Check that this is a real multicast address
    if (!QHostAddress(groupAddress).isMulticast())
        return false;

    // ) Insert in the set and emit signal to say the multicast list changed
    _multicastGroups.insert(groupAddress);
    Q_EMIT multicastGroupsChanged();

    // ) This function should be overriden to really join the multicast interface if possible
    return true;
}

bool AbstractServer::leaveMulticastGroup(const QString& groupAddress)
{
    const auto it = _multicastGroups.find(groupAddress);

    // ) Is the multicast group present
    if (it == _multicastGroups.end())
        return false;

    // ) Remove the multicast address from the list, then emit a signal to say the list changed
    _multicastGroups.erase(it);
    Q_EMIT multicastGroupsChanged();

    // ) You should override this function to really implement the leave of the group
    return true;
}

bool AbstractServer::leaveAllMulticastGroups()
{
    bool allSuccess = true;
    while(!_multicastGroups.empty())
    {
        const auto group = _multicastGroups.begin();
        if (!leaveMulticastGroup(*group))
            allSuccess = false;
    }
    return allSuccess;
}

bool AbstractServer::isMulticastGroupPresent(const QString& groupAddress)
{
    return _multicastGroups.find(groupAddress) != _multicastGroups.end();
}

void AbstractServer::clearRxCounter()
{
    resetRxPacketsPerSeconds();
    resetRxPacketsTotal();
    resetRxBytesPerSeconds();
    resetRxBytesTotal();
}

void AbstractServer::clearTxCounter()
{
    resetTxPacketsPerSeconds();
    resetTxPacketsTotal();
    resetTxBytesPerSeconds();
    resetTxBytesTotal();
}

void AbstractServer::clearRxInvalidCounter()
{
    resetRxInvalidPacketTotal();
}

void AbstractServer::clearCounters()
{
    clearRxCounter();
    clearTxCounter();
}
