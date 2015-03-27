/*
    Copyright 2013 Jan Grulich <jgrulich@redhat.com>
    Copyright 2013, 2014 Lukas Tinkl <ltinkl@redhat.com>

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

#include "connectiondetaileditor.h"
#include "ui_connectiondetaileditor.h"

#include "settings/bondwidget.h"
#include "settings/bridgewidget.h"
#include "settings/btwidget.h"
#include "settings/cdmawidget.h"
#include "settings/connectionwidget.h"
#include "settings/gsmwidget.h"
#include "settings/infinibandwidget.h"
#include "settings/ipv4widget.h"
#include "settings/ipv6widget.h"
#include "settings/pppwidget.h"
#include "settings/pppoewidget.h"
#include "settings/vlanwidget.h"
#include "settings/wimaxwidget.h"
#include "settings/wiredsecurity.h"
#include "settings/wiredconnectionwidget.h"
#include "settings/wifisecurity.h"
#include "settings/wificonnectionwidget.h"
#include "vpn/openvpn/nm-openvpn-service.h"
#include "vpnuiplugin.h"

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/AdslSetting>
#include <NetworkManagerQt/CdmaSetting>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/PppoeSetting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessDevice>

#include <QDebug>

#include <KUser>
#include <KPluginFactory>
#include <KServiceTypeTrader>
#include <QPushButton>
#include <KWallet/Wallet>

using namespace NetworkManager;

ConnectionDetailEditor::ConnectionDetailEditor(NetworkManager::ConnectionSettings::ConnectionType type, QWidget* parent,
                                               const QString &masterUuid, const QString &slaveType, Qt::WindowFlags f):
    QDialog(parent, f),
    m_ui(new Ui::ConnectionDetailEditor),
    m_connection(new NetworkManager::ConnectionSettings(type)),
    m_new(true),
    m_masterUuid(masterUuid),
    m_slaveType(slaveType)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_ui->setupUi(this);

    initEditor();
}

ConnectionDetailEditor::ConnectionDetailEditor(NetworkManager::ConnectionSettings::ConnectionType type, QWidget* parent,
                                               const QString &vpnType, bool shared, Qt::WindowFlags f):
    QDialog(parent, f),
    m_ui(new Ui::ConnectionDetailEditor),
    m_connection(new NetworkManager::ConnectionSettings(type)),
    m_new(true),
    m_vpnType(vpnType)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_ui->setupUi(this);

    if (shared) {
        if (type == ConnectionSettings::Wireless) {
            NetworkManager::WirelessSetting::Ptr wifiSetting = m_connection->setting(Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            wifiSetting->setMode(WirelessSetting::Adhoc);
            wifiSetting->setSsid(i18n("my_shared_connection").toUtf8());

            foreach (const NetworkManager::Device::Ptr & device, NetworkManager::networkInterfaces()) {
                if (device->type() == Device::Wifi) {
                    NetworkManager::WirelessDevice::Ptr wifiDev = device.objectCast<NetworkManager::WirelessDevice>();
                    if (wifiDev) {
                        if (wifiDev->wirelessCapabilities().testFlag(WirelessDevice::ApCap)) {
                            wifiSetting->setMode(WirelessSetting::Ap);
                            wifiSetting->setMacAddress(Utils::macAddressFromString(wifiDev->hardwareAddress()));
                        }
                    }
                }
            }
        }
        NetworkManager::Ipv4Setting::Ptr ipv4Setting = m_connection->setting(Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
        ipv4Setting->setMethod(Ipv4Setting::Shared);
        m_connection->setAutoconnect(false);
    }

    initEditor();
}

ConnectionDetailEditor::ConnectionDetailEditor(ConnectionSettings::ConnectionType type, const QVariantList &args, QWidget *parent, Qt::WindowFlags f):
    QDialog(parent, f),
    m_ui(new Ui::ConnectionDetailEditor),
    m_connection(new NetworkManager::ConnectionSettings(type)),
    m_new(true)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_ui->setupUi(this);

    // parse args given from the wizard
    qDebug() << "Editing new mobile connection, number of args:" << args.count();
    foreach(const QVariant & arg, args) {
        qDebug() << "Argument:" << arg;
    }

    if (args.count() == 2) { //GSM or CDMA
        QVariantMap tmp = qdbus_cast<QVariantMap>(args.value(1));

#if 0 // network IDs are not used yet and seem to break the setting
        if (args.count() == 3) { // gsm specific
            QStringList networkIds = args.value(1).toStringList();
            if (!networkIds.isEmpty())
                tmp.insert("network-id", networkIds.first());
        }
#endif

        m_connection->setConnectionType(type);
        m_connection->setId(args.value(0).toString());
        qDebug() << "New " << m_connection->typeAsString(m_connection->connectionType()) << "connection initializing with:" << tmp;
        if (type == ConnectionSettings::Gsm)
            m_connection->setting(Setting::Gsm)->fromMap(tmp);
        else if (type == ConnectionSettings::Cdma)
            m_connection->setting(Setting::Cdma)->fromMap(tmp);
        else
            qWarning() << Q_FUNC_INFO << "Unhandled setting type";

        qDebug() << "New connection initialized:";
        qDebug() << *m_connection;
    } else {
        qWarning() << Q_FUNC_INFO << "Unexpected number of args to parse";
    }

    initEditor();
}


ConnectionDetailEditor::ConnectionDetailEditor(const NetworkManager::ConnectionSettings::Ptr &connection, QWidget* parent, Qt::WindowFlags f, bool newConnection):
    QDialog(parent, f),
    m_ui(new Ui::ConnectionDetailEditor),
    m_connection(connection),
    m_new(newConnection),
    m_masterUuid(connection->master()),
    m_slaveType(connection->slaveType())
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_ui->setupUi(this);

    initEditor();
}

ConnectionDetailEditor::~ConnectionDetailEditor()
{
    m_connection.clear();
    delete m_ui;
}

void ConnectionDetailEditor::initEditor()
{
    enableOKButton(false);

    if (!m_new) {
        NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(m_connection->uuid());
        if (connection) {
            bool hasSecrets = false;
            connect(connection.data(), SIGNAL(gotSecrets(QString,bool,NMVariantMapMap,QString)),
                    SLOT(gotSecrets(QString,bool,NMVariantMapMap,QString)), Qt::UniqueConnection);

            if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Adsl) {
                NetworkManager::AdslSetting::Ptr adslSetting = connection->settings()->setting(NetworkManager::Setting::Adsl).staticCast<NetworkManager::AdslSetting>();
                if (adslSetting && !adslSetting->needSecrets().isEmpty()) {
                    hasSecrets = true;
                    connection->secrets("adsl");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Bluetooth) {
                NetworkManager::GsmSetting::Ptr gsmSetting = connection->settings()->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                if (gsmSetting && !gsmSetting->needSecrets().isEmpty()) {
                    hasSecrets = true;
                    connection->secrets("gsm");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Cdma) {
                NetworkManager::CdmaSetting::Ptr cdmaSetting = connection->settings()->setting(NetworkManager::Setting::Cdma).staticCast<NetworkManager::CdmaSetting>();
                if (cdmaSetting && !cdmaSetting->needSecrets().isEmpty()) {
                    hasSecrets = true;
                    connection->secrets("cdma");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Gsm) {
                NetworkManager::GsmSetting::Ptr gsmSetting = connection->settings()->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                if (gsmSetting && !gsmSetting->needSecrets().isEmpty()) {
                    hasSecrets = true;
                    connection->secrets("gsm");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Pppoe) {
                NetworkManager::PppoeSetting::Ptr pppoeSetting = connection->settings()->setting(NetworkManager::Setting::Pppoe).staticCast<NetworkManager::PppoeSetting>();
                if (pppoeSetting && !pppoeSetting->needSecrets().isEmpty()) {
                    hasSecrets = true;
                    connection->secrets("pppoe");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Wired) {
                NetworkManager::Security8021xSetting::Ptr securitySetting = connection->settings()->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>();
                if (securitySetting && !securitySetting->needSecrets().isEmpty()) {
                    hasSecrets = true;
                    connection->secrets("802-1x");
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
                NetworkManager::WirelessSecuritySetting::Ptr wifiSecuritySetting = connection->settings()->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
                if (wifiSecuritySetting &&
                    (wifiSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaEap ||
                    (wifiSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WirelessSecuritySetting::Ieee8021x &&
                     wifiSecuritySetting->authAlg() != WirelessSecuritySetting::Leap))) {
                    NetworkManager::Security8021xSetting::Ptr securitySetting = connection->settings()->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>();
                    if (securitySetting && !securitySetting->needSecrets().isEmpty()) {
                        hasSecrets = true;
                        connection->secrets("802-1x");
                    }
                } else {
                    if (!wifiSecuritySetting->needSecrets().isEmpty()) {
                        hasSecrets = true;
                        connection->secrets("802-11-wireless-security");
                    }
                }
            } else if (m_connection->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
                hasSecrets = true;
                connection->secrets("vpn");
            }

            if (!hasSecrets) {
                initTabs();
            }
        }
    } else {
        initTabs();
    }

    if (m_connection->id().isEmpty()) {
        setWindowTitle(i18n("New Connection (%1)", m_connection->typeAsString(m_connection->connectionType())));
        m_ui->connectionName->setText(i18n("New %1 connection", m_connection->typeAsString(m_connection->connectionType())));
    } else {
        setWindowTitle(i18n("Edit Connection '%1'", m_connection->id()));
        m_ui->connectionName->setText(m_connection->id());
    }

    connect(this, SIGNAL(accepted()), SLOT(saveSetting()));
    connect(this, SIGNAL(accepted()), SLOT(disconnectSignals()));
    connect(this, SIGNAL(rejected()), SLOT(disconnectSignals()));
}

void ConnectionDetailEditor::initTabs()
{
    if (m_new) {
        m_connection->addToPermissions(KUser().loginName(), QString());
    }

    // create/set UUID, need this beforehand for slave connections
    QString uuid = m_connection->uuid();
    if (QUuid(uuid).isNull()) {
        uuid = NetworkManager::ConnectionSettings::createNewUuid();
        m_connection->setUuid(uuid);
    }

    ConnectionWidget * connectionWidget = new ConnectionWidget(m_connection);
    m_ui->tabWidget->addTab(connectionWidget, i18nc("General", "General configuration"));

    qDebug() << "Initting tabs, UUID:" << uuid;

    const NetworkManager::ConnectionSettings::ConnectionType type = m_connection->connectionType();

    // setup the widget tabs
    QString serviceType;
    if (type == NetworkManager::ConnectionSettings::Wired) {
        WiredConnectionWidget * wiredWidget = new WiredConnectionWidget(m_connection->setting(NetworkManager::Setting::Wired), this);
        m_ui->tabWidget->addTab(wiredWidget, i18n("Wired"));
        WiredSecurity * wiredSecurity = new WiredSecurity(m_connection->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>(), this);
        m_ui->tabWidget->addTab(wiredSecurity, i18n("802.1x Security"));
    } else if (type == NetworkManager::ConnectionSettings::Wireless) {
        WifiConnectionWidget * wifiWidget = new WifiConnectionWidget(m_connection->setting(NetworkManager::Setting::Wireless), this);
        m_ui->tabWidget->addTab(wifiWidget, i18n("Wireless"));
        WifiSecurity * wifiSecurity = new WifiSecurity(m_connection->setting(NetworkManager::Setting::WirelessSecurity),
                                                       m_connection->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>(),
                                                       this);
        m_ui->tabWidget->addTab(wifiSecurity, i18n("Wireless Security"));
    } else if (type == NetworkManager::ConnectionSettings::Pppoe) { // DSL
        PppoeWidget * pppoeWidget = new PppoeWidget(m_connection->setting(NetworkManager::Setting::Pppoe), this);
        m_ui->tabWidget->addTab(pppoeWidget, i18n("DSL"));
        WiredConnectionWidget * wiredWidget = new WiredConnectionWidget(m_connection->setting(NetworkManager::Setting::Wired), this);
        m_ui->tabWidget->addTab(wiredWidget, i18n("Wired"));
    } else if (type == NetworkManager::ConnectionSettings::Gsm) { // GSM
        GsmWidget * gsmWidget = new GsmWidget(m_connection->setting(NetworkManager::Setting::Gsm), this);
        m_ui->tabWidget->addTab(gsmWidget, i18n("Mobile Broadband (%1)", m_connection->typeAsString(m_connection->connectionType())));
    } else if (type == NetworkManager::ConnectionSettings::Cdma) { // CDMA
        CdmaWidget * cdmaWidget = new CdmaWidget(m_connection->setting(NetworkManager::Setting::Cdma), this);
        m_ui->tabWidget->addTab(cdmaWidget, i18n("Mobile Broadband (%1)", m_connection->typeAsString(m_connection->connectionType())));
    } else if (type == NetworkManager::ConnectionSettings::Bluetooth) {  // Bluetooth
        BtWidget * btWidget = new BtWidget(m_connection->setting(NetworkManager::Setting::Bluetooth), this);
        m_ui->tabWidget->addTab(btWidget, i18n("Bluetooth"));
        NetworkManager::BluetoothSetting::Ptr btSetting = m_connection->setting(NetworkManager::Setting::Bluetooth).staticCast<NetworkManager::BluetoothSetting>();
        if (btSetting->profileType() == NetworkManager::BluetoothSetting::Dun) {
            GsmWidget * gsmWidget = new GsmWidget(m_connection->setting(NetworkManager::Setting::Gsm), this);
            m_ui->tabWidget->addTab(gsmWidget, i18n("GSM"));
            PPPWidget * pppWidget = new PPPWidget(m_connection->setting(NetworkManager::Setting::Ppp), this);
            m_ui->tabWidget->addTab(pppWidget, i18n("PPP"));
        }
    } else if (type == NetworkManager::ConnectionSettings::Infiniband) { // Infiniband
        InfinibandWidget * infinibandWidget = new InfinibandWidget(m_connection->setting(NetworkManager::Setting::Infiniband), this);
        m_ui->tabWidget->addTab(infinibandWidget, i18n("Infiniband"));
    } else if (type == NetworkManager::ConnectionSettings::Bond) { // Bond
        BondWidget * bondWidget = new BondWidget(uuid, m_connection->setting(NetworkManager::Setting::Bond), this);
        m_ui->tabWidget->addTab(bondWidget, i18n("Bond"));
    } else if (type == NetworkManager::ConnectionSettings::Bridge) { // Bridge
        BridgeWidget * bridgeWidget = new BridgeWidget(uuid, m_connection->setting(NetworkManager::Setting::Bridge), this);
        m_ui->tabWidget->addTab(bridgeWidget, i18n("Bridge"));
    } else if (type == NetworkManager::ConnectionSettings::Vlan) { // Vlan
        VlanWidget * vlanWidget = new VlanWidget(m_connection->setting(NetworkManager::Setting::Vlan), this);
        m_ui->tabWidget->addTab(vlanWidget, i18n("Vlan"));
    } else if (type == NetworkManager::ConnectionSettings::Wimax) { // Wimax
        WimaxWidget * wimaxWidget = new WimaxWidget(m_connection->setting(NetworkManager::Setting::Wimax), this);
        m_ui->tabWidget->addTab(wimaxWidget, i18n("WiMAX"));
    } else if (type == NetworkManager::ConnectionSettings::Vpn) { // VPN
        QString error;
        VpnUiPlugin * vpnPlugin = 0;
        NetworkManager::VpnSetting::Ptr vpnSetting =
                m_connection->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
        if (!vpnSetting) {
            qDebug() << "Missing VPN setting!";
        } else {
            if (m_new && !m_vpnType.isEmpty()) {
                serviceType = m_vpnType;
                vpnSetting->setServiceType(serviceType);
            }
            else
                serviceType = vpnSetting->serviceType();
            //qDebug() << "Editor loading VPN plugin" << serviceType;
            //vpnSetting->printSetting();
            vpnPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>(QString::fromLatin1("PlasmaNetworkManagement/VpnUiPlugin"),
                                                                                 QString::fromLatin1("[X-NetworkManager-Services]=='%1'").arg(serviceType),
                                                                                 this, QVariantList(), &error);
            if (vpnPlugin && error.isEmpty()) {
                const QString shortName = serviceType.section('.', -1);
                SettingWidget * vpnWidget = vpnPlugin->widget(vpnSetting, this);
                m_ui->tabWidget->addTab(vpnWidget, i18n("VPN (%1)", shortName));
            } else {
                qDebug() << error << ", serviceType == " << serviceType;
            }
        }
    }

    // PPP widget
    if (type == ConnectionSettings::Pppoe || type == ConnectionSettings::Cdma || type == ConnectionSettings::Gsm) {
        PPPWidget * pppWidget = new PPPWidget(m_connection->setting(NetworkManager::Setting::Ppp), this);
        m_ui->tabWidget->addTab(pppWidget, i18n("PPP"));
    }

    // IPv4 widget
    if (!isSlave()) {
        IPv4Widget * ipv4Widget = new IPv4Widget(m_connection->setting(NetworkManager::Setting::Ipv4), this);
        m_ui->tabWidget->addTab(ipv4Widget, i18n("IPv4"));
    }

    // IPv6 widget
    if ((type == ConnectionSettings::Wired
         || type == ConnectionSettings::Wireless
         || type == ConnectionSettings::Infiniband
         || type == ConnectionSettings::Wimax
         || type == ConnectionSettings::Bond
         || type == ConnectionSettings::Bridge
         || type == ConnectionSettings::Vlan
         || (type == ConnectionSettings::Vpn && serviceType == QLatin1String(NM_DBUS_SERVICE_OPENVPN))) && !isSlave()) {
        IPv6Widget * ipv6Widget = new IPv6Widget(m_connection->setting(NetworkManager::Setting::Ipv6), this);
        m_ui->tabWidget->addTab(ipv6Widget, i18n("IPv6"));
    }

    // validation
    bool valid = true;
    for (int i = 0; i < m_ui->tabWidget->count(); ++i) {
        SettingWidget * widget = dynamic_cast<SettingWidget *>(m_ui->tabWidget->widget(i));
        if (widget) {
            valid = valid && widget->isValid();
            connect(widget, SIGNAL(validChanged(bool)), SLOT(validChanged(bool)));
        }
    }
    enableOKButton(valid);
    m_ui->tabWidget->setCurrentIndex(1);

    KAcceleratorManager::manage(this);
}

void ConnectionDetailEditor::saveSetting()
{
    ConnectionWidget * connectionWidget = static_cast<ConnectionWidget*>(m_ui->tabWidget->widget(0));

    NMVariantMapMap settings = connectionWidget->setting();

    bool agentOwned = false;
    // We can store secrets into KWallet if KWallet is enabled and permissions list is not empty
    if (!settings.value("connection").value("permissions").toStringList().isEmpty() && KWallet::Wallet::isEnabled()) {
        agentOwned = true;
    }

    for (int i = 1; i < m_ui->tabWidget->count(); ++i) {
        SettingWidget * widget = static_cast<SettingWidget*>(m_ui->tabWidget->widget(i));
        const QString type = widget->type();
        if (type != NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x) &&
            type != NetworkManager::Setting::typeAsString(NetworkManager::Setting::WirelessSecurity)) {
            settings.insert(type, widget->setting(agentOwned));
        }

        // add 802.1x security if needed
        QVariantMap security8021x;
        if (type == NetworkManager::Setting::typeAsString(NetworkManager::Setting::WirelessSecurity)) {
            WifiSecurity * wifiSecurity = static_cast<WifiSecurity*>(widget);
            if (wifiSecurity->enabled()) {
                settings.insert(type, wifiSecurity->setting(agentOwned));
            }
            if (wifiSecurity->enabled8021x()) {
                security8021x = static_cast<WifiSecurity *>(widget)->setting8021x(agentOwned);
                settings.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x), security8021x);
            }
        } else if (type == NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x)) {
            WiredSecurity * wiredSecurity = static_cast<WiredSecurity*>(widget);
            if (wiredSecurity->enabled8021x()) {
                security8021x = static_cast<WiredSecurity *>(widget)->setting(agentOwned);
                settings.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x), security8021x);
            }
        }
    }

    NetworkManager::ConnectionSettings * connectionSettings = new NetworkManager::ConnectionSettings(m_connection->connectionType());

    connectionSettings->fromMap(settings);
    connectionSettings->setId(m_ui->connectionName->text());

    if (connectionSettings->connectionType() == ConnectionSettings::Wireless) {
        NetworkManager::WirelessSecuritySetting::Ptr securitySetting = connectionSettings->setting(Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
        NetworkManager::WirelessSetting::Ptr wirelessSetting = connectionSettings->setting(Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();

        if (securitySetting && wirelessSetting) {
            if (securitySetting->keyMgmt() != NetworkManager::WirelessSecuritySetting::WirelessSecuritySetting::Unknown) {
                wirelessSetting->setSecurity("802-11-wireless-security");
            }
        }
    }

    // set UUID
    connectionSettings->setUuid(m_connection->uuid());

    // set master & slave type
    if (isSlave()) {
        connectionSettings->setMaster(m_masterUuid);
        connectionSettings->setSlaveType(m_slaveType);
    }

    qDebug() << *connectionSettings;  // debug

    if (m_new) { // create new connection
        connect(NetworkManager::settingsNotifier(), SIGNAL(connectionAddComplete(QString,bool,QString)),
                SLOT(connectionAddComplete(QString,bool,QString)));
        NetworkManager::addConnection(connectionSettings->toMap());
    } else {  // update existing connection
        NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(connectionSettings->uuid());

        if (connection) {
            connection->update(connectionSettings->toMap());
        }
    }
}

void ConnectionDetailEditor::connectionAddComplete(const QString& id, bool success, const QString& msg)
{
    qDebug() << id << " - " << success << " - " << msg;
}

void ConnectionDetailEditor::gotSecrets(const QString& id, bool success, const NMVariantMapMap& secrets, const QString& msg)
{
    if (id == m_connection->uuid() && success) {
        foreach (const QString & key, secrets.keys()) {
            NetworkManager::Setting::Ptr setting = m_connection->setting(NetworkManager::Setting::typeFromString(key));
            if (setting) {
                setting->secretsFromMap(secrets.value(key));
            }
        }
    }

    initTabs();
}

void ConnectionDetailEditor::disconnectSignals()
{
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(m_connection->uuid());

    if (connection) {
        disconnect(connection.data(), SIGNAL(gotSecrets(QString,bool,NMVariantMapMap,QString)),
                   this, SLOT(gotSecrets(QString,bool,NMVariantMapMap,QString)));
    }
}

void ConnectionDetailEditor::validChanged(bool valid)
{
    if (!valid) {
        enableOKButton(false);
        return;
    } else {
        for (int i = 1; i < m_ui->tabWidget->count(); ++i) {
            SettingWidget * widget = static_cast<SettingWidget*>(m_ui->tabWidget->widget(i));
            if (!widget->isValid()) {
                enableOKButton(false);
                return;
            }
        }
    }

    enableOKButton(true);
}

void ConnectionDetailEditor::enableOKButton(bool enabled)
{
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}
