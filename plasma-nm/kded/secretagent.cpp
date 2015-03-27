/*
    Copyright 2013 Jan Grulich <jgrulich@redhat.com>
    Copyright 2013 Lukas Tinkl <ltinkl@redhat.com>
    Copyright 2013 by Daniel Nicoletti <dantti12@gmail.com>

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

#include "secretagent.h"
#include "passworddialog.h"

#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>

#include <QStringBuilder>

#include <KPluginFactory>
#include <KWindowSystem>
#include <KDialog>
#include <KDebug>
#include <KConfig>
#include <KConfigGroup>
#include <KWallet/Wallet>
#include <QDebug>

SecretAgent::SecretAgent(QObject* parent)
    : NetworkManager::SecretAgent("org.kde.networkmanagement", parent)
    , m_openWalletFailed(false)
    , m_wallet(0)
    , m_dialog(0)
{
    connect(NetworkManager::notifier(), SIGNAL(serviceDisappeared()),
            this, SLOT(killDialogs()));

    // We have to import secrets previously stored in plaintext files
    importSecretsFromPlainTextFiles();
}

SecretAgent::~SecretAgent()
{
}

NMVariantMapMap SecretAgent::GetSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path, const QString &setting_name,
                                        const QStringList &hints, uint flags)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "Path:" << connection_path.path();
    qDebug() << "Setting name:" << setting_name;
    qDebug() << "Hints:" << hints;
    qDebug() << "Flags:" << flags;

    QString callId = connection_path.path() % setting_name;
    foreach (const SecretsRequest & request, m_calls) {
        if (request == callId) {
            kWarning() << "GetSecrets was called again! This should not happen, cancelling first call" << connection_path.path() << setting_name;
            CancelGetSecrets(connection_path, setting_name);
            break;
        }
    }

    setDelayedReply(true);
    SecretsRequest request(SecretsRequest::GetSecrets);
    request.callId = callId;
    request.connection = connection;
    request.connection_path = connection_path;
    request.flags = static_cast<NetworkManager::SecretAgent::GetSecretsFlags>(flags);
    request.hints = hints;
    request.setting_name = setting_name;
    request.message = message();
    m_calls << request;

    processNext();

    return NMVariantMapMap();
}

void SecretAgent::SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    kDebug() << connection_path.path();

    setDelayedReply(true);
    SecretsRequest::Type type;
    if (hasSecrets(connection)) {
        type = SecretsRequest::SaveSecrets;
    } else {
        type = SecretsRequest::DeleteSecrets;
    }
    SecretsRequest request(type);
    request.connection = connection;
    request.connection_path = connection_path;
    request.message = message();
    m_calls << request;

    processNext();
}

void SecretAgent::DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connection_path)
{
    kDebug() << connection_path.path();

    setDelayedReply(true);
    SecretsRequest request(SecretsRequest::DeleteSecrets);
    request.connection = connection;
    request.connection_path = connection_path;
    request.message = message();
    m_calls << request;

    processNext();
}

void SecretAgent::CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name)
{
    kDebug() << connection_path.path() << setting_name;
    QString callId = connection_path.path() % setting_name;
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls.at(i);
        if (request.type == SecretsRequest::GetSecrets && callId == request.callId) {
            if (m_dialog == request.dialog) {
                m_dialog = 0;
            }
            delete request.dialog;
            sendError(SecretAgent::AgentCanceled,
                      QLatin1String("Agent canceled the password dialog"),
                      request.message);
            m_calls.removeAt(i);
            break;
        }
    }

    processNext();
}

void SecretAgent::dialogAccepted()
{
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets && request.dialog == m_dialog) {
            NMVariantMapMap connection = request.dialog->secrets();
            sendSecrets(connection, request.message);
            NetworkManager::ConnectionSettings::Ptr connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(connection));
            NetworkManager::ConnectionSettings::Ptr completeConnectionSettings;
            NetworkManager::Connection::Ptr con = NetworkManager::findConnectionByUuid(connectionSettings->uuid());
            if (con) {
                completeConnectionSettings = con->settings();
            } else {
                completeConnectionSettings = connectionSettings;
            }
            if (request.saveSecretsWithoutReply && completeConnectionSettings->connectionType() != NetworkManager::ConnectionSettings::Vpn) {
                bool requestOffline = true;
                if (completeConnectionSettings->connectionType() == NetworkManager::ConnectionSettings::Gsm) {
                    NetworkManager::GsmSetting::Ptr gsmSetting = completeConnectionSettings->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();
                    if (gsmSetting) {
                        if (gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved) ||
                            gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                            requestOffline = false;
                        } else if (gsmSetting->pinFlags().testFlag(NetworkManager::Setting::NotSaved) ||
                                   gsmSetting->pinFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                            requestOffline = false;
                        }
                    }
                } else if (completeConnectionSettings->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
                    NetworkManager::WirelessSetting::Ptr wirelessSetting = completeConnectionSettings->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
                    if (wirelessSetting && !wirelessSetting->security().isEmpty()) {
                        NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting = completeConnectionSettings->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
                        if (wirelessSecuritySetting && wirelessSecuritySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaEap) {
                            NetworkManager::Security8021xSetting::Ptr security8021xSetting = completeConnectionSettings->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>();
                            if (security8021xSetting) {
                                if (security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodFast) ||
                                    security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodTtls) ||
                                    security8021xSetting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethodPeap)) {
                                    if (security8021xSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved) ||
                                        security8021xSetting->passwordFlags().testFlag(NetworkManager::Setting::NotRequired)) {
                                        requestOffline = false;
                                    }
                                }
                            }
                        }
                    }
                }

                if (requestOffline) {
                    SecretsRequest requestOffline(SecretsRequest::SaveSecrets);
                    requestOffline.connection = connection;
                    requestOffline.connection_path = request.connection_path;
                    requestOffline.saveSecretsWithoutReply = true;
                    m_calls << requestOffline;
                }
            }

            m_calls.removeAt(i);
            break;
        }
    }

    m_dialog->deleteLater();
    m_dialog = 0;

    processNext();
}

void SecretAgent::dialogRejected()
{
    for (int i = 0; i < m_calls.size(); ++i) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets && request.dialog == m_dialog) {
            sendError(SecretAgent::UserCanceled,
                      QLatin1String("User canceled the password dialog"),
                      request.message);
            m_calls.removeAt(i);
            break;
        }
    }

    m_dialog->deleteLater();
    m_dialog = 0;

    processNext();
}

void SecretAgent::killDialogs()
{
    int i = 0;
    while (i < m_calls.size()) {
        SecretsRequest request = m_calls[i];
        if (request.type == SecretsRequest::GetSecrets) {
            delete request.dialog;
            m_calls.removeAt(i);
        }

        ++i;
    }
}

void SecretAgent::walletOpened(bool success)
{
    if (!success) {
        m_openWalletFailed = true;
        m_wallet->deleteLater();
        m_wallet = 0;
    } else {
        m_openWalletFailed = false;
    }

    processNext();
}

void SecretAgent::walletClosed()
{
    if (m_wallet) {
        m_wallet->deleteLater();
    }
    m_wallet = 0;
}

void SecretAgent::processNext()
{
    int i = 0;
    while (i < m_calls.size()) {
        SecretsRequest &request = m_calls[i];
        switch (request.type) {
        case SecretsRequest::GetSecrets:
            if (processGetSecrets(request)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        case SecretsRequest::SaveSecrets:
            if (processSaveSecrets(request)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        case SecretsRequest::DeleteSecrets:
            if (processDeleteSecrets(request)) {
                m_calls.removeAt(i);
                continue;
            }
            break;
        }
        ++i;
    }
}

bool SecretAgent::processGetSecrets(SecretsRequest &request) const
{
    if (m_dialog) {
        return false;
    }

    NetworkManager::ConnectionSettings connectionSettings(request.connection);

    NetworkManager::Setting::Ptr setting = connectionSettings.setting(request.setting_name);

    const bool requestNew = request.flags & RequestNew;
    const bool userRequested = request.flags & UserRequested;
    const bool allowInteraction = request.flags & AllowInteraction;
    const bool isVpn = (setting->type() == NetworkManager::Setting::Vpn);

    NMStringMap secretsMap;
    if (!requestNew && useWallet()) {
        if (m_wallet->isOpen()) {
            if (m_wallet->hasFolder("Network Management") && m_wallet->setFolder("Network Management")) {
                QString key = QLatin1Char('{') % connectionSettings.uuid() % QLatin1Char('}') % QLatin1Char(';') % request.setting_name;
                m_wallet->readMap(key, secretsMap);
            }
        } else {
            kDebug() << "Waiting for the wallet to open";
            return false;
        }
    }

    if (!secretsMap.isEmpty()) {
        setting->secretsFromStringMap(secretsMap);
        if (!isVpn && setting->needSecrets(requestNew).isEmpty()) {
            // Enough secrets were retrieved from storage
            request.connection[request.setting_name] = setting->secretsToMap();
            sendSecrets(request.connection, request.message);
            return true;
        }
    }

    if (requestNew || (allowInteraction && !setting->needSecrets(requestNew).isEmpty()) || (allowInteraction && userRequested) || (isVpn && allowInteraction)) {
        m_dialog = new PasswordDialog(request.connection, request.flags, request.setting_name);
        connect(m_dialog, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
        connect(m_dialog, SIGNAL(rejected()), this, SLOT(dialogRejected()));
        if (isVpn) {
            m_dialog->setupVpnUi(connectionSettings);
        } else {
            m_dialog->setupGenericUi(connectionSettings);
        }

        if (m_dialog->hasError()) {
            sendError(m_dialog->error(),
                      m_dialog->errorMessage(),
                      request.message);
            delete m_dialog;
            m_dialog = 0;
            return true;
        } else {
            request.dialog = m_dialog;
            request.saveSecretsWithoutReply = !connectionSettings.permissions().isEmpty();
            m_dialog->show();
            KWindowSystem::setState(m_dialog->winId(), NET::KeepAbove);
            KWindowSystem::forceActiveWindow(m_dialog->winId());
            return false;
        }
    } else if (isVpn && userRequested) { // just return what we have
        NMVariantMapMap result;
        NetworkManager::VpnSetting::Ptr vpnSetting;
        vpnSetting = connectionSettings.setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
        result.insert("vpn", vpnSetting->secretsToMap());
        sendSecrets(result, request.message);
        return true;
    } else if (setting->needSecrets().isEmpty()) {
        NMVariantMapMap result;
        result.insert(setting->name(), setting->secretsToMap());
        sendSecrets(result, request.message);
        return true;
    } else {
        sendError(SecretAgent::InternalError,
                  QLatin1String("Plasma-nm did not know how to handle the request"),
                  request.message);
        return true;
    }
}

bool SecretAgent::processSaveSecrets(SecretsRequest &request) const
{
    if (useWallet()) {
        if (m_wallet->isOpen()) {
            NetworkManager::ConnectionSettings connectionSettings(request.connection);

            if (!m_wallet->hasFolder("Network Management")) {
                m_wallet->createFolder("Network Management");
            }

            if (m_wallet->setFolder("Network Management")) {
                foreach (const NetworkManager::Setting::Ptr &setting, connectionSettings.settings()) {
                    NMStringMap secretsMap = setting->secretsToStringMap();

                    if (!secretsMap.isEmpty()) {
                        QString entryName = QLatin1Char('{') % connectionSettings.uuid() % QLatin1Char('}') % QLatin1Char(';') % setting->name();
                        m_wallet->writeMap(entryName, secretsMap);
                    }
                }
            } else if (!request.saveSecretsWithoutReply) {
                sendError(SecretAgent::InternalError,
                          QLatin1String("Could not store secrets in the wallet."),
                          request.message);
                return true;
            }
        } else {
            kDebug() << "Waiting for the wallet to open";
            return false;
        }
    }

    if (!request.saveSecretsWithoutReply) {
        QDBusMessage reply = request.message.createReply();
        if (!QDBusConnection::systemBus().send(reply)) {
            kWarning() << "Failed put save secrets reply into the queue";
        }
    }

    return true;
}

bool SecretAgent::processDeleteSecrets(SecretsRequest &request) const
{
    if (useWallet()) {
        if (m_wallet->isOpen()) {
            if (m_wallet->hasFolder("Network Management") && m_wallet->setFolder("Network Management")) {
                NetworkManager::ConnectionSettings connectionSettings(request.connection);
                foreach (const NetworkManager::Setting::Ptr &setting, connectionSettings.settings()) {
                    QString entryName = QLatin1Char('{') % connectionSettings.uuid() % QLatin1Char('}') % QLatin1Char(';') % setting->name();
                    foreach (const QString &entry, m_wallet->entryList()) {
                        if (entry.startsWith(entryName)) {
                            m_wallet->removeEntry(entryName);
                        }
                    }
                }
            }
        } else {
            kDebug() << "Waiting for the wallet to open";
            return false;
        }
    }

    QDBusMessage reply = request.message.createReply();
    if (!QDBusConnection::systemBus().send(reply)) {
        kWarning() << "Failed put delete secrets reply into the queue";
    }

    return true;
}

bool SecretAgent::useWallet() const
{
    if (m_wallet) {
        return true;
    }

    /* If opening of KWallet failed before, we should not try to open it again and
     * we should return false instead */
    if (m_openWalletFailed) {
        m_openWalletFailed = false;
        return false;
    }

    if (KWallet::Wallet::isEnabled()) {
        m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0, KWallet::Wallet::Asynchronous);
        if (m_wallet) {
            connect(m_wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpened(bool)));
            connect(m_wallet, SIGNAL(walletClosed()), this, SLOT(walletClosed()));
            return true;
        } else {
            kWarning() << "Error opening kwallet.";
        }
    } else if (m_wallet) {
        m_wallet->deleteLater();
        m_wallet = 0;
    }

    return false;
}

bool SecretAgent::hasSecrets(const NMVariantMapMap &connection) const
{
    NetworkManager::ConnectionSettings connectionSettings(connection);
    foreach (const NetworkManager::Setting::Ptr &setting, connectionSettings.settings()) {
        if (!setting->secretsToMap().isEmpty()) {
            return true;
        }
    }

    return false;
}

void SecretAgent::sendSecrets(const NMVariantMapMap &secrets, const QDBusMessage &message) const
{
    QDBusMessage reply;
    reply = message.createReply(QVariant::fromValue(secrets));
    if (!QDBusConnection::systemBus().send(reply)) {
        kWarning() << "Failed put the secret into the queue";
    }
}

void SecretAgent::importSecretsFromPlainTextFiles()
{
    KConfig config(QLatin1String("plasma-networkmanagement"), KConfig::SimpleConfig);

    // No action is required when the list of secrets is empty
    if (!config.groupList().isEmpty()) {
        foreach (const QString &groupName, config.groupList()) {
            QString loadedUuid = groupName.split(';').first().remove('{').remove('}');
            QString loadedSettingType = groupName.split(';').last();
            NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(loadedUuid);
            if (connection) {
                NetworkManager::Setting::SecretFlags secretFlags = KWallet::Wallet::isEnabled() ? NetworkManager::Setting::AgentOwned : NetworkManager::Setting::None;
                QMap<QString, QString> secrets = config.entryMap(groupName);
                NMVariantMapMap settings = connection->settings()->toMap();

                foreach (const QString &setting, settings.keys()) {
                    if (setting == QLatin1String("vpn")) {
                        NetworkManager::VpnSetting::Ptr vpnSetting = connection->settings()->setting(NetworkManager::Setting::Vpn).staticCast<NetworkManager::VpnSetting>();
                        if (vpnSetting) {
                            // Add loaded secrets from the config file
                            vpnSetting->secretsFromStringMap(secrets);

                            NMStringMap vpnData = vpnSetting->data();
                            // Reset flags, we can't save secrets to our secret agent when KWallet is not enabled, because
                            // we dropped support for plaintext files, therefore they need to be stored to NetworkManager
                            foreach (const QString &key, vpnData.keys()) {
                                if (key.endsWith(QLatin1String("-flags"))) {
                                    vpnData.insert(key, QString::number((int)secretFlags));
                                }
                            }

                            vpnSetting->setData(vpnData);
                            settings.insert(setting, vpnSetting->toMap());
                            connection->update(settings);
                        }
                    } else {
                        if (setting == loadedSettingType) {
                            QVariantMap tmpSetting = settings.value(setting);
                            // Reset flags, we can't save secrets to our secret agent when KWallet is not enabled, because
                            // we dropped support for plaintext files, therefore they need to be stored to NetworkManager
                            foreach (const QString &key, tmpSetting.keys()) {
                                if (key.endsWith(QLatin1String("-flags"))) {
                                    tmpSetting.insert(key, (int)secretFlags);
                                }
                            }

                            // Add loaded secrets from the config file
                            QMap<QString, QString>::const_iterator it = secrets.constBegin();
                            QMap<QString, QString>::const_iterator end = secrets.constEnd();
                            for (; it != end; ++it) {
                                tmpSetting.insert(it.key(), it.value());
                            }

                            // Replace the old setting with the new one
                            settings.insert(setting, tmpSetting);
                            // Update the connection which re-saves secrets
                            connection->update(settings);
                        }
                    }
                }
            }

            // Remove the group
            KConfigGroup group(&config, groupName);
            group.deleteGroup();
        }
    }
}
