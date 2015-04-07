/*
    Copyright 2013 Lukas Tinkl <ltinkl@redhat.com>
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

#include "connectioneditor.h"
#include "ui_connectioneditor.h"
#include "connectiondetaileditor.h"
#include "editoridentitymodel.h"
#include "editorproxymodel.h"
#include "networkmodel.h"
#include "mobileconnectionwizard.h"
#include "uiutils.h"
#include "vpnuiplugin.h"
#include <networkmodelitem.h>

#include <KActionCollection>
#include <KLocale>
#include <KMessageBox>
#include <KService>
#include <KServiceTypeTrader>
#include <KStandardAction>
#include <KAction>
#include <KXMLGUIFactory>
#include <KMenu>
#include <KAcceleratorManager>
#include <KConfig>
#include <KConfigGroup>
#include <KWallet/Wallet>
#include <KStandardDirs>
#include <KFileDialog>
#include <KShell>
#include <KFilterProxySearchLine>

#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/VpnSetting>

using namespace NetworkManager;

ConnectionEditor::ConnectionEditor(QWidget* parent, Qt::WindowFlags flags)
    : KXmlGuiWindow(parent, flags)
    , m_editor(new Ui::ConnectionEditor)
    , m_handler(new Handler(this))
{
    QWidget * tmp = new QWidget(this);
    m_editor->setupUi(tmp);
    setCentralWidget(tmp);

    m_editor->connectionsWidget->setSortingEnabled(false);
    m_editor->connectionsWidget->sortByColumn(0, Qt::AscendingOrder);
    m_editor->connectionsWidget->setSortingEnabled(true);
    m_editor->connectionsWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_editor->messageWidget->hide();
    m_editor->messageWidget->setCloseButtonVisible(false);
    m_editor->messageWidget->setWordWrap(true);

    m_editor->ktreewidgetsearchline->lineEdit()->setClickMessage(i18n("Type here to search connections..."));

    initializeConnections();
    initializeMenu();

    connect(m_editor->connectionsWidget, SIGNAL(pressed(QModelIndex)),
            SLOT(slotItemClicked(QModelIndex)));
    connect(m_editor->connectionsWidget, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(slotItemDoubleClicked(QModelIndex)));
    connect(m_editor->connectionsWidget, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(slotContextMenuRequested(QPoint)));
    connect(m_menu->menu(), SIGNAL(triggered(QAction*)),
            SLOT(addConnection(QAction*)));
    connect(NetworkManager::settingsNotifier(), SIGNAL(connectionAdded(QString)),
            SLOT(connectionAdded(QString)));

    KConfig config("kde-nm-connection-editor");
    KConfigGroup generalGroup = config.group("General");

    if (generalGroup.isValid()) {
        if (generalGroup.readEntry("FirstStart", true)) {
            importSecretsFromPlainTextFiles();
        }

        generalGroup.writeEntry("FirstStart", false);
    }
}

ConnectionEditor::~ConnectionEditor()
{
    delete m_editor;
}

void ConnectionEditor::initializeMenu()
{
    m_menu = new KActionMenu(KIcon("list-add"), i18n("Add"), this);
    m_menu->menu()->setSeparatorsCollapsible(false);
    m_menu->setDelayed(false);

    QAction * action = m_menu->addSeparator();
    action->setText(i18n("Hardware"));

    // TODO Adsl
    action = new QAction(i18n("DSL"), this);
    action->setData(NetworkManager::ConnectionSettings::Pppoe);
    m_menu->addAction(action);
    action = new QAction(i18n("InfiniBand"), this);
    action->setData(NetworkManager::ConnectionSettings::Infiniband);
    m_menu->addAction(action);
#if WITH_MODEMMANAGER_SUPPORT
    action = new QAction(i18n("Mobile Broadband..."), this);
    action->setData(NetworkManager::ConnectionSettings::Gsm);
    m_menu->addAction(action);
#endif
    action = new QAction(i18n("Wired"), this);
    action->setData(NetworkManager::ConnectionSettings::Wired);
    action->setProperty("shared", false);
    m_menu->addAction(action);
    action = new QAction(i18n("Wired (shared)"), this);
    action->setData(NetworkManager::ConnectionSettings::Wired);
    action->setProperty("shared", true);
    m_menu->addAction(action);
    action = new QAction(i18n("Wireless"), this);
    action->setData(NetworkManager::ConnectionSettings::Wireless);
    action->setProperty("shared", false);
    m_menu->addAction(action);
    action = new QAction(i18n("Wireless (shared)"), this);
    action->setData(NetworkManager::ConnectionSettings::Wireless);
    action->setProperty("shared", true);
    m_menu->addAction(action);
    action = new QAction(i18n("WiMAX"), this);
    action->setData(NetworkManager::ConnectionSettings::Wimax);
    m_menu->addAction(action);

    action = m_menu->addSeparator();
    action->setText(i18nc("Virtual hardware devices, eg Bridge, Bond", "Virtual"));

    action = new QAction(i18n("Bond"), this);
    action->setData(NetworkManager::ConnectionSettings::Bond);
    m_menu->addAction(action);
    action = new QAction(i18n("Bridge"), this);
    action->setData(NetworkManager::ConnectionSettings::Bridge);
    m_menu->addAction(action);
    action = new QAction(i18n("VLAN"), this);
    action->setData(NetworkManager::ConnectionSettings::Vlan);
    m_menu->addAction(action);

    action = m_menu->addSeparator();
    action->setText(i18n("VPN"));

    const KService::List services = KServiceTypeTrader::self()->query("PlasmaNetworkManagement/VpnUiPlugin");
    foreach (const KService::Ptr & service, services) {
        qDebug() << "Found VPN plugin" << service->name() << ", type:" << service->property("X-NetworkManager-Services", QVariant::String).toString();

        action = new QAction(service->name(), this);
        action->setData(NetworkManager::ConnectionSettings::Vpn);
        action->setProperty("type", service->property("X-NetworkManager-Services", QVariant::String));
        m_menu->addAction(action);
    }

    actionCollection()->addAction("add_connection", m_menu);

    KAction * kAction = new KAction(KIcon("network-connect"), i18n("Connect"), this);
    kAction->setEnabled(false);
    connect(kAction, SIGNAL(triggered()), this, SLOT(connectConnection()));
    actionCollection()->addAction("connect_connection", kAction);

    kAction = new KAction(KIcon("network-disconnect"), i18n("Disconnect"), this);
    kAction->setEnabled(false);
    connect(kAction, SIGNAL(triggered()), this, SLOT(disconnectConnection()));
    actionCollection()->addAction("disconnect_connection", kAction);

    kAction = new KAction(KIcon("configure"), i18n("Edit..."), this);
    kAction->setEnabled(false);
    connect(kAction, SIGNAL(triggered()), SLOT(editConnection()));
    actionCollection()->addAction("edit_connection", kAction);

    kAction = new KAction(KIcon("edit-delete"), i18n("Delete"), this);
    kAction->setEnabled(false);
    kAction->setShortcut(Qt::Key_Delete);
    connect(kAction, SIGNAL(triggered()), SLOT(removeConnection()));
    actionCollection()->addAction("delete_connection", kAction);

    kAction = new KAction(KIcon("document-import"), i18n("Import VPN..."), this);
    actionCollection()->addAction("import_vpn", kAction);
    connect(kAction, SIGNAL(triggered()), SLOT(importVpn()));

    kAction = new KAction(KIcon("document-export"), i18n("Export VPN..."), this);
    actionCollection()->addAction("export_vpn", kAction);
    kAction->setEnabled(false);
    connect(kAction, SIGNAL(triggered()), SLOT(exportVpn()));

    KStandardAction::keyBindings(guiFactory(), SLOT(configureShortcuts()), actionCollection());
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    setupGUI(QSize(480, 480));

    setAutoSaveSettings();

    KAcceleratorManager::manage(this);
}

void ConnectionEditor::addConnection(QAction* action)
{
    qDebug() << "ADDING new connection" << action->data().toUInt();
    const QString vpnType = action->property("type").toString();
    qDebug() << "VPN type:" << vpnType;

    ConnectionSettings::ConnectionType type = static_cast<ConnectionSettings::ConnectionType>(action->data().toUInt());

    if (type == NetworkManager::ConnectionSettings::Gsm) { // launch the mobile broadband wizard, both gsm/cdma
#if WITH_MODEMMANAGER_SUPPORT
        QWeakPointer<MobileConnectionWizard> wizard = new MobileConnectionWizard(NetworkManager::ConnectionSettings::Unknown, this);
        if (wizard.data()->exec() == QDialog::Accepted && wizard.data()->getError() == MobileProviders::Success) {
            qDebug() << "Mobile broadband wizard finished:" << wizard.data()->type() << wizard.data()->args();
            QPointer<ConnectionDetailEditor> editor = new ConnectionDetailEditor(wizard.data()->type(), wizard.data()->args(), this);
            editor->exec();

            if (editor) {
                editor->deleteLater();
            }
        }
        if (wizard) {
            wizard.data()->deleteLater();
        }
#endif
    } else {
        bool shared = false;
        if (type == ConnectionSettings::Wired || type == ConnectionSettings::Wireless) {
            shared = action->property("shared").toBool();
        }

        QPointer<ConnectionDetailEditor> editor = new ConnectionDetailEditor(type, this, vpnType, shared);
        editor->exec();

        if (editor) {
            editor->deleteLater();
        }
    }
}

void ConnectionEditor::connectionAdded(const QString& connection)
{
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);

    if (!con) {
        return;
    }

    if (con->settings()->isSlave())
        return;

    m_editor->messageWidget->animatedShow();
    m_editor->messageWidget->setMessageType(KMessageWidget::Positive);
    m_editor->messageWidget->setText(i18n("Connection %1 has been added", con->name()));
    QTimer::singleShot(5000, m_editor->messageWidget, SLOT(animatedHide()));
}

void ConnectionEditor::connectConnection()
{
    const QModelIndex currentIndex = m_editor->connectionsWidget->currentIndex();

    if (!currentIndex.isValid() || currentIndex.parent().isValid()) {
        return;
    }

    const QString connectionPath = currentIndex.data(NetworkModel::ConnectionPathRole).toString();
    const QString devicePath = currentIndex.data(NetworkModel::DevicePathRole).toString();
    const QString specificPath = currentIndex.data(NetworkModel::SpecificPathRole).toString();

    m_handler->activateConnection(connectionPath, devicePath, specificPath);
}

void ConnectionEditor::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    QModelIndex currentIndex = m_editor->connectionsWidget->currentIndex();
    if (currentIndex.isValid()) {
        for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
            QModelIndex index = m_editor->connectionsWidget->model()->index(i, 0);
            if (index.isValid() && index == currentIndex) {
                // Re-check enabled/disabled actions
                slotItemClicked(currentIndex);
                break;
            }
        }
    }
}

void ConnectionEditor::disconnectConnection()
{
    const QModelIndex currentIndex = m_editor->connectionsWidget->currentIndex();

    if (!currentIndex.isValid() || currentIndex.parent().isValid()) {
        return;
    }

    const QString connectionPath = currentIndex.data(NetworkModel::ConnectionPathRole).toString();
    const QString devicePath = currentIndex.data(NetworkModel::DevicePathRole).toString();
    m_handler->deactivateConnection(connectionPath, devicePath);
}

void ConnectionEditor::editConnection()
{
    const QModelIndex currentIndex = m_editor->connectionsWidget->currentIndex();

    if (!currentIndex.isValid() || currentIndex.parent().isValid()) {
        return;
    }

    slotItemDoubleClicked(currentIndex);
}

void ConnectionEditor::initializeConnections()
{
    EditorIdentityModel * model = new EditorIdentityModel(this);
    EditorProxyModel * filterModel = new EditorProxyModel(this);
    filterModel->setSourceModel(model);
    connect(filterModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),SLOT(dataChanged(QModelIndex,QModelIndex)));

    m_editor->connectionsWidget->setModel(filterModel);
    m_editor->ktreewidgetsearchline->setProxy(filterModel);
    m_editor->connectionsWidget->header()->setResizeMode(0, QHeaderView::Stretch);
}

void ConnectionEditor::removeConnection()
{
    const QModelIndex currentIndex = m_editor->connectionsWidget->currentIndex();

    if (!currentIndex.isValid() || currentIndex.parent().isValid()) {
        return;
    }

    Connection::Ptr connection = NetworkManager::findConnectionByUuid(currentIndex.data(NetworkModel::UuidRole).toString());

    if (!connection) {
        return;
    }

    if (KMessageBox::questionYesNo(this, i18n("Do you want to remove the connection '%1'?", connection->name()), i18n("Remove Connection"), KStandardGuiItem::remove(),
                                   KStandardGuiItem::no(), QString(), KMessageBox::Dangerous)
            == KMessageBox::Yes) {
        foreach (const NetworkManager::Connection::Ptr &con, NetworkManager::listConnections()) {
            NetworkManager::ConnectionSettings::Ptr settings = con->settings();
            if (settings->master() == connection->uuid()) {
                con->remove();
            }
        }
        connection->remove();
    }
}

void ConnectionEditor::slotContextMenuRequested(const QPoint&)
{
    QMenu * menu = new QMenu(this);

    QModelIndex index = m_editor->connectionsWidget->currentIndex();
    const bool isActive = (NetworkManager::ActiveConnection::State)index.data(NetworkModel::ConnectionStateRole).toUInt() == NetworkManager::ActiveConnection::Activated;
    const bool isAvailable = (NetworkModelItem::ItemType)index.data(NetworkModel::ItemTypeRole).toUInt() == NetworkModelItem::AvailableConnection;

    if (isAvailable && !isActive) {
        menu->addAction(actionCollection()->action("connect_connection"));
    } else if (isAvailable && isActive) {
        menu->addAction(actionCollection()->action("disconnect_connection"));
    }
    menu->addAction(actionCollection()->action("edit_connection"));
    menu->addAction(actionCollection()->action("delete_connection"));
    menu->exec(QCursor::pos());
}

void ConnectionEditor::slotItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    qDebug() << "Clicked item" << index.data(NetworkModel::UuidRole).toString();

    if (index.parent().isValid()) { // category
        actionCollection()->action("connect_connection")->setEnabled(false);
        actionCollection()->action("disconnect_connection")->setEnabled(false);
        actionCollection()->action("edit_connection")->setEnabled(false);
        actionCollection()->action("delete_connection")->setEnabled(false);
        actionCollection()->action("export_vpn")->setEnabled(false);
        actionCollection()->action("export_vpn")->setEnabled(false);
    } else {                       //connection
        const bool isActive = (NetworkManager::ActiveConnection::State)index.data(NetworkModel::ConnectionStateRole).toUInt() == NetworkManager::ActiveConnection::Activated;
        const bool isActivating = (NetworkManager::ActiveConnection::State)index.data(NetworkModel::ConnectionStateRole).toUInt() == NetworkManager::ActiveConnection::Activating;
        const bool isAvailable = (NetworkModelItem::ItemType)index.data(NetworkModel::ItemTypeRole).toUInt() == NetworkModelItem::AvailableConnection;

        actionCollection()->action("connect_connection")->setEnabled(isAvailable && !isActive && !isActivating);
        actionCollection()->action("disconnect_connection")->setEnabled(isAvailable && (isActive || isActivating));
        actionCollection()->action("edit_connection")->setEnabled(true);
        actionCollection()->action("delete_connection")->setEnabled(true);
        const bool isVpn = static_cast<NetworkManager::ConnectionSettings::ConnectionType>(index.data(NetworkModel::TypeRole).toUInt()) ==
                           NetworkManager::ConnectionSettings::Vpn;
        actionCollection()->action("export_vpn")->setEnabled(isVpn);
    }
}

void ConnectionEditor::slotItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    qDebug() << "Double clicked item" << index.data(NetworkModel::UuidRole).toString();

    if (index.parent().isValid()) { // category
        qDebug() << "double clicked on the root item which is not editable";
        return;
    }

    const QString uuid = index.data(NetworkModel::UuidRole).toString();
    m_handler->editConnection(uuid);
}

void ConnectionEditor::importSecretsFromPlainTextFiles()
{
    const QString secretsDirectory = KStandardDirs::locateLocal("data", "networkmanagement/secrets/");
    QDir dir(secretsDirectory);
    if (dir.exists() && !dir.entryList(QDir::Files).isEmpty()) {
        QMap<QString, QMap<QString, QString > > resultingMap;
        foreach (const QString & file, dir.entryList(QDir::Files)) {
            KConfig config(secretsDirectory % file, KConfig::SimpleConfig);
            foreach (const QString & groupName, config.groupList()) {
                KConfigGroup group = config.group(groupName);
                QMap<QString, QString> map = group.entryMap();
                if (!map.isEmpty()) {
                    const QString entry = file % ';' % groupName;
                    resultingMap.insert(entry, map);
                }
            }
        }

        storeSecrets(resultingMap);
    }
}

void ConnectionEditor::storeSecrets(const QMap< QString, QMap< QString, QString > >& map)
{
    if (KWallet::Wallet::isEnabled()) {
        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0, KWallet::Wallet::Synchronous);

        if (!wallet || !wallet->isOpen()) {
            return;
        }

        if (!wallet->hasFolder("Network Management")) {
            wallet->createFolder("Network Management");
        }

        if (wallet->hasFolder("Network Management") && wallet->setFolder("Network Management")) {
            int count = 0;
            foreach (const QString & entry, map.keys()) {
                QString connectionUuid = entry.split(';').first();
                connectionUuid.replace('{',"").replace('}',"");
                NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(connectionUuid);

                if (connection) {
                    wallet->writeMap(entry, map.value(entry));
                    ++count;
                }
            }
        }
    }
}

void ConnectionEditor::importVpn()
{
    // get the list of supported extensions
    const KService::List services = KServiceTypeTrader::self()->query("PlasmaNetworkManagement/VpnUiPlugin");
    QString extensions;
    foreach (const KService::Ptr &service, services) {
        VpnUiPlugin * vpnPlugin = service->createInstance<VpnUiPlugin>(this);
        if (vpnPlugin) {
            extensions += vpnPlugin->supportedFileExtensions() % QLatin1Literal(" ");
            delete vpnPlugin;
        }
    }

    const QString filename = KFileDialog::getOpenFileName(KUrl(), extensions.simplified(), this, i18n("Import VPN Connection"));
    if (!filename.isEmpty()) {
        QFileInfo fi(filename);
        const QString ext = QLatin1Literal("*.") % fi.suffix();
        qDebug() << "Importing VPN connection" << filename << "extension:" << ext;

        foreach (const KService::Ptr &service, services) {
            VpnUiPlugin * vpnPlugin = service->createInstance<VpnUiPlugin>(this);
            if (vpnPlugin && vpnPlugin->supportedFileExtensions().contains(ext)) {
                qDebug() << "Found VPN plugin" << service->name() << ", type:" << service->property("X-NetworkManager-Services", QVariant::String).toString();

                NMVariantMapMap connection = vpnPlugin->importConnectionSettings(filename);

                //qDebug() << "Raw connection:" << connection;

                NetworkManager::ConnectionSettings connectionSettings;
                connectionSettings.fromMap(connection);
                connectionSettings.setUuid(NetworkManager::ConnectionSettings::createNewUuid());

                //qDebug() << "Converted connection:" << connectionSettings;

                const QString conId = NetworkManager::addConnection(connectionSettings.toMap());
                qDebug() << "Adding imported connection under id:" << conId;

                if (connection.isEmpty()) { // the "positive" part will arrive with connectionAdded
                    m_editor->messageWidget->animatedShow();
                    m_editor->messageWidget->setMessageType(KMessageWidget::Error);
                    m_editor->messageWidget->setText(i18n("Importing VPN connection %1 failed\n%2", fi.fileName(), vpnPlugin->lastErrorMessage()));
                    QTimer::singleShot(5000, m_editor->messageWidget, SLOT(animatedHide()));
                } else {
                    delete vpnPlugin;
                    break; // stop iterating over the plugins if the import produced at least some output
                }

                delete vpnPlugin;
            }
        }
    }
}

void ConnectionEditor::exportVpn()
{
    const QModelIndex currentIndex = m_editor->connectionsWidget->currentIndex();

    if (!currentIndex.isValid() || currentIndex.parent().isValid()) {
        return;
    }

    Connection::Ptr connection = NetworkManager::findConnectionByUuid(currentIndex.data(NetworkModel::UuidRole).toString());
    if (!connection) {
        return;
    }

    NetworkManager::ConnectionSettings::Ptr connSettings = connection->settings();

    if (connSettings->connectionType() != NetworkManager::ConnectionSettings::Vpn)
        return;

    NetworkManager::VpnSetting::Ptr vpnSetting = connSettings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();

    qDebug() << "Exporting VPN connection" << connection->name() << "type:" << vpnSetting->serviceType();

    QString error;
    VpnUiPlugin * vpnPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>(QString::fromLatin1("PlasmaNetworkManagement/VpnUiPlugin"),
                                                                                       QString::fromLatin1("[X-NetworkManager-Services]=='%1'").arg(vpnSetting->serviceType()),
                                                                                       this, QVariantList(), &error);

    if (vpnPlugin) {
        if (vpnPlugin->suggestedFileName(connSettings).isEmpty()) { // this VPN doesn't support export
            m_editor->messageWidget->animatedShow();
            m_editor->messageWidget->setMessageType(KMessageWidget::Error);
            m_editor->messageWidget->setText(i18n("Export is not supported by this VPN type"));
            QTimer::singleShot(5000, m_editor->messageWidget, SLOT(animatedHide()));
            return;
        }

        const KUrl url = KUrl::fromLocalFile(KGlobalSettings::documentPath() + QDir::separator() + vpnPlugin->suggestedFileName(connSettings));
        const QString filename = KFileDialog::getSaveFileName(url, vpnPlugin->supportedFileExtensions(), this, i18n("Export VPN Connection"));
        if (!filename.isEmpty()) {
            if (!vpnPlugin->exportConnectionSettings(connSettings, filename)) {
                m_editor->messageWidget->animatedShow();
                m_editor->messageWidget->setMessageType(KMessageWidget::Error);
                m_editor->messageWidget->setText(i18n("Exporting VPN connection %1 failed\n%2", connection->name(), vpnPlugin->lastErrorMessage()));
                QTimer::singleShot(5000, m_editor->messageWidget, SLOT(animatedHide()));
            } else {
                m_editor->messageWidget->animatedShow();
                m_editor->messageWidget->setMessageType(KMessageWidget::Positive);
                m_editor->messageWidget->setText(i18n("VPN connection %1 exported successfully", connection->name()));
                QTimer::singleShot(5000, m_editor->messageWidget, SLOT(animatedHide()));
            }
        }
        delete vpnPlugin;
    } else {
        qWarning() << "Error getting VpnUiPlugin for export:" << error;
    }
}
