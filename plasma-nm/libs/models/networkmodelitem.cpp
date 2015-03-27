/*
    Copyright 2013-2014 Jan Grulich <jgrulich@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "networkmodelitem.h"
#include "globalconfig.h"
#include "uiutils.h"

#include <NetworkManagerQt/BluetoothDevice>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/ModemDevice>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnConnection>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WimaxDevice>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>

#include <KGlobal>
#include <KLocale>
#include <KLocalizedString>

#include <Plasma/DataEngineManager>

#if WITH_MODEMMANAGER_SUPPORT
#include <ModemManagerQt/manager.h>
#include <ModemManagerQt/modem.h>
#endif

NetworkModelItem::NetworkModelItem(QObject* parent)
    : QObject(parent)
    , m_connectionState(NetworkManager::ActiveConnection::Deactivated)
    , m_deviceState(NetworkManager::Device::UnknownState)
    , m_duplicate(false)
    , m_mode(NetworkManager::WirelessSetting::Infrastructure)
    , m_securityType(NetworkManager::Utils::None)
    , m_signal(0)
    , m_slave(false)
    , m_type(NetworkManager::ConnectionSettings::Unknown)
    , m_vpnState(NetworkManager::VpnConnection::Unknown)
{
}

NetworkModelItem::NetworkModelItem(const NetworkModelItem* item, QObject* parent)
    : QObject(parent)
    , m_connectionPath(item->connectionPath())
    , m_connectionState(NetworkManager::ActiveConnection::Deactivated)
    , m_duplicate(true)
    , m_mode(item->mode())
    , m_name(item->name())
    , m_securityType(item->securityType())
    , m_slave(item->slave())
    , m_ssid(item->ssid())
    , m_timestamp(item->timestamp())
    , m_type(item->type())
    , m_uuid(item->uuid())
    , m_vpnState(NetworkManager::VpnConnection::Unknown)
{
}

NetworkModelItem::~NetworkModelItem()
{
}

QString NetworkModelItem::activeConnectionPath() const
{
    return m_activeConnectionPath;
}

void NetworkModelItem::setActiveConnectionPath(const QString& path)
{
    m_activeConnectionPath = path;
}

QString NetworkModelItem::connectionPath() const
{
    return m_connectionPath;
}

void NetworkModelItem::setConnectionPath(const QString& path)
{
    m_connectionPath = path;
}

NetworkManager::ActiveConnection::State NetworkModelItem::connectionState() const
{
    return m_connectionState;
}

void NetworkModelItem::setConnectionState(NetworkManager::ActiveConnection::State state)
{
    m_connectionState = state;
}

QString NetworkModelItem::details() const
{
    return m_details;
}

QString NetworkModelItem::devicePath() const
{
    return m_devicePath;
}

QString NetworkModelItem::deviceName() const
{
    return m_deviceName;
}

void NetworkModelItem::setDeviceName(const QString& name)
{
    m_deviceName = name;
}

void NetworkModelItem::setDevicePath(const QString& path)
{
    m_devicePath = path;
}

QString NetworkModelItem::deviceState() const
{
    return UiUtils::connectionStateToString(m_deviceState);
}

void NetworkModelItem::setDeviceState(const NetworkManager::Device::State state)
{
    m_deviceState = state;
}

bool NetworkModelItem::duplicate() const
{
    return m_duplicate;
}

QString NetworkModelItem::icon() const
{
    switch (m_type) {
        case NetworkManager::ConnectionSettings::Adsl:
            return "network-mobile-100";
            break;
        case NetworkManager::ConnectionSettings::Bluetooth:
            if (connectionState() == NetworkManager::ActiveConnection::Activated) {
                return "network-bluetooth-activated";
            } else {
                return "network-bluetooth";
            }
            break;
        case NetworkManager::ConnectionSettings::Bond:
            break;
        case NetworkManager::ConnectionSettings::Bridge:
            break;
        case NetworkManager::ConnectionSettings::Cdma:
        case NetworkManager::ConnectionSettings::Gsm:
            if (m_signal == 0 ) {
                return "network-mobile-0";
            } else if (m_signal < 20) {
                return "network-mobile-20";
            } else if (m_signal < 40) {
                return "network-mobile-40";
            } else if (m_signal < 60) {
                return "network-mobile-60";
            } else if (m_signal < 80) {
                return "network-mobile-80";
            } else {
                return "network-mobile-100";
            }
            break;
        case NetworkManager::ConnectionSettings::Infiniband:
            break;
        case NetworkManager::ConnectionSettings::OLPCMesh:
            break;
        case NetworkManager::ConnectionSettings::Pppoe:
            return "network-mobile-100";
            break;
        case NetworkManager::ConnectionSettings::Vlan:
            break;
        case NetworkManager::ConnectionSettings::Vpn:
            return "network-vpn";
            break;
        case NetworkManager::ConnectionSettings::Wimax:
            if (m_signal == 0 ) {
                return "network-wireless-0";
            } else if (m_signal < 20) {
                return "network-wireless-20";
            } else if (m_signal < 40) {
                return "network-wireless-40";
            } else if (m_signal < 60) {
                return "network-wireless-60";
            } else if (m_signal < 80) {
                return "network-wireless-80";
            } else {
                return "network-wireless-100";
            }
            break;
        case NetworkManager::ConnectionSettings::Wired:
            if (connectionState() == NetworkManager::ActiveConnection::Activated) {
                return "network-wired-activated";
            } else {
                return "network-wired";
            }
            break;
        case NetworkManager::ConnectionSettings::Wireless:
            if (m_signal == 0 ) {
                if (m_mode == NetworkManager::WirelessSetting::Adhoc || m_mode == NetworkManager::WirelessSetting::Ap) {
                    return (m_securityType <= NetworkManager::Utils::None) ? "network-wireless-100" : "network-wireless-100-locked";
                }
                return (m_securityType <= NetworkManager::Utils::None) ? "network-wireless-0" : "network-wireless-0-locked";
            } else if (m_signal < 20) {
                return (m_securityType <= NetworkManager::Utils::None) ? "network-wireless-20" : "network-wireless-20-locked";
            } else if (m_signal < 40) {
                return (m_securityType <= NetworkManager::Utils::None) ? "network-wireless-40" : "network-wireless-40-locked";
            } else if (m_signal < 60) {
                return (m_securityType <= NetworkManager::Utils::None) ? "network-wireless-60" : "network-wireless-60-locked";
            } else if (m_signal < 80) {
                return (m_securityType <= NetworkManager::Utils::None) ? "network-wireless-80" : "network-wireless-80-locked";
            } else {
                return (m_securityType <= NetworkManager::Utils::None) ? "network-wireless-100" : "network-wireless-100-locked";
            }
            break;
        default:
            break;
    }

    if (connectionState() == NetworkManager::ActiveConnection::Activated) {
        return "network-wired-activated";
    } else {
        return "network-wired";
    }
}

NetworkModelItem::ItemType NetworkModelItem::itemType() const
{
    if (!m_devicePath.isEmpty() ||
        m_type == NetworkManager::ConnectionSettings::Bond ||
        m_type == NetworkManager::ConnectionSettings::Bridge ||
        m_type == NetworkManager::ConnectionSettings::Vlan ||
        ((NetworkManager::status() == NetworkManager::Connected ||
          NetworkManager::status() == NetworkManager::ConnectedLinkLocal ||
          NetworkManager::status() == NetworkManager::ConnectedSiteOnly) && m_type == NetworkManager::ConnectionSettings::Vpn)) {
        if (m_connectionPath.isEmpty() && m_type == NetworkManager::ConnectionSettings::Wireless) {
            return NetworkModelItem::AvailableAccessPoint;
        } else if (m_connectionPath.isEmpty() && m_type == NetworkManager::ConnectionSettings::Wimax) {
            return NetworkModelItem::AvailableNsp;
        } else {
            return NetworkModelItem::AvailableConnection;
        }
    }
    return NetworkModelItem::UnavailableConnection;
}

NetworkManager::WirelessSetting::NetworkMode NetworkModelItem::mode() const
{
    return m_mode;
}

void NetworkModelItem::setMode(const NetworkManager::WirelessSetting::NetworkMode mode)
{
    m_mode = mode;
}

QString NetworkModelItem::name() const
{
    return m_name;
}

void NetworkModelItem::setName(const QString& name)
{
    m_name = name;
}

QString NetworkModelItem::nsp() const
{
    return m_nsp;
}

void NetworkModelItem::setNsp(const QString& nsp)
{
    m_nsp = nsp;
}

QString NetworkModelItem::originalName() const
{
    if (m_deviceName.isEmpty()) {
        return m_name;
    }
    return m_name + " (" + m_deviceName + ')';
}

QString NetworkModelItem::sectionType() const
{
    if (m_connectionState == NetworkManager::ActiveConnection::Activated) {
        return i18n("Active connections");
    }  else {
        return i18n("Available connections");
    }
}

NetworkManager::Utils::WirelessSecurityType NetworkModelItem::securityType() const
{
    return m_securityType;
}

void NetworkModelItem::setSecurityType(NetworkManager::Utils::WirelessSecurityType type)
{
    m_securityType = type;
}

int NetworkModelItem::signal() const
{
    return m_signal;
}

void NetworkModelItem::setSignal(int signal)
{
    m_signal = signal;
}

bool NetworkModelItem::slave() const
{
    return m_slave;
}

void NetworkModelItem::setSlave(bool slave)
{
    m_slave = slave;
}

QString NetworkModelItem::specificPath() const
{
    return m_specificPath;
}

void NetworkModelItem::setSpecificPath(const QString& path)
{
    m_specificPath = path;
}

QString NetworkModelItem::ssid() const
{
    return m_ssid;
}

void NetworkModelItem::setSsid(const QString& ssid)
{
    m_ssid = ssid;
}

NetworkManager::ConnectionSettings::ConnectionType NetworkModelItem::type() const
{
    return m_type;
}

QDateTime NetworkModelItem::timestamp() const
{
    return m_timestamp;
}

void NetworkModelItem::setTimestamp(const QDateTime& date)
{
    m_timestamp = date;
}

void NetworkModelItem::setType(NetworkManager::ConnectionSettings::ConnectionType type)
{
    m_type = type;
}

QString NetworkModelItem::uni() const
{
    if (m_type == NetworkManager::ConnectionSettings::Wireless && m_uuid.isEmpty()) {
        return m_ssid + '%' + m_devicePath;
    } else if (m_type == NetworkManager::ConnectionSettings::Wimax && m_uuid.isEmpty()) {
        return m_nsp + '%' + m_devicePath;
    } else {
        return m_connectionPath + '%' + m_devicePath;
    }
}

QString NetworkModelItem::uuid() const
{
    return m_uuid;
}

void NetworkModelItem::setUuid(const QString& uuid)
{
    m_uuid = uuid;
}

QString NetworkModelItem::vpnState() const
{
    return UiUtils::vpnConnectionStateToString(m_vpnState);
}

void NetworkModelItem::setVpnState(NetworkManager::VpnConnection::State state)
{
    m_vpnState = state;
}

bool NetworkModelItem::operator==(const NetworkModelItem* item) const
{
    if (!item->uuid().isEmpty() && !uuid().isEmpty()) {
        if (item->devicePath() == devicePath() && item->uuid() == uuid()) {
            return true;
        }
    } else if (item->type() == NetworkManager::ConnectionSettings::Wireless && type() == NetworkManager::ConnectionSettings::Wireless) {
        if (item->ssid() == ssid() && item->devicePath() == devicePath()) {
            return true;
        }
    } else if (item->type() == NetworkManager::ConnectionSettings::Wimax && type() == NetworkManager::ConnectionSettings::Wimax) {
        if (item->nsp() == nsp() && item->devicePath() == devicePath()) {
            return true;
        }
    }

    return false;
}

void NetworkModelItem::updateDetails()
{
    if (itemType() == NetworkModelItem::UnavailableConnection) {
        return;
    }

    m_details = "<qt><table>";

    QStringList detailsList = GlobalConfig().detailKeys();

    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(m_connectionPath);
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(m_devicePath);

    m_details += UiUtils::connectionDetails(device, connection, detailsList);

    if (m_type == NetworkManager::ConnectionSettings::Bluetooth) {
        NetworkManager::BluetoothDevice::Ptr btDevice = device.objectCast<NetworkManager::BluetoothDevice>();
        m_details += UiUtils::bluetoothDetails(btDevice, detailsList);
    } else if (m_type == NetworkManager::ConnectionSettings::Gsm || m_type == NetworkManager::ConnectionSettings::Cdma) {
        NetworkManager::ModemDevice::Ptr modemDevice = device.objectCast<NetworkManager::ModemDevice>();
        m_details += UiUtils::modemDetails(modemDevice, detailsList);
    } else if (m_type == NetworkManager::ConnectionSettings::Wimax) {
        NetworkManager::WimaxNsp::Ptr wimaxNsp;
        NetworkManager::WimaxDevice::Ptr wimaxDevice = device.objectCast<NetworkManager::WimaxDevice>();
        wimaxNsp = wimaxDevice->findNsp(m_specificPath);
        if (wimaxDevice && wimaxNsp) {
            m_details += UiUtils::wimaxDetails(wimaxDevice, wimaxNsp, connection, detailsList);
        }
    } else if (m_type == NetworkManager::ConnectionSettings::Wired) {
        NetworkManager::WiredDevice::Ptr wiredDevice;
        if (device) {
            wiredDevice = device.objectCast<NetworkManager::WiredDevice>();
        }
        m_details += UiUtils::wiredDetails(wiredDevice, connection, detailsList);
    } else if (m_type == NetworkManager::ConnectionSettings::Wireless) {
        NetworkManager::WirelessDevice::Ptr wirelessDevice;
        if (device) {
            wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
        }
        NetworkManager::AccessPoint::Ptr ap;
        if (wirelessDevice) {
            ap = wirelessDevice->findAccessPoint(m_specificPath);
        }
        m_details += UiUtils::wirelessDetails(wirelessDevice, ap, connection, detailsList);
    } else if (m_type == NetworkManager::ConnectionSettings::Vpn) {
        NetworkManager::ActiveConnection::Ptr active = NetworkManager::findActiveConnection(m_activeConnectionPath);
        NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(m_connectionPath);
        NetworkManager::ConnectionSettings::Ptr connectionSettings;
        NetworkManager::VpnSetting::Ptr vpnSetting;
        NetworkManager::VpnConnection::Ptr vpnConnection;

        if (connection) {
            connectionSettings = connection->settings();
        }
        if (connectionSettings) {
            vpnSetting = connectionSettings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
        }

        if (active) {
            vpnConnection = NetworkManager::VpnConnection::Ptr(new NetworkManager::VpnConnection(active->path()), &QObject::deleteLater);
        }
        m_details += UiUtils::vpnDetails(vpnConnection, vpnSetting, detailsList);
    }

    m_details += "</table></qt>";
}
