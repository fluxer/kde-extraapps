/* This file is part of the KDE project

   Copyright (C) 2008 - 2009 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>
   Copyright (C) 2011 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "httpserver.h"

#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include "core/kget.h"
#include "settings.h"

#include <KGlobalSettings>
#include <KStandardDirs>
#include <KDebug>

#include <QFile>
#include <QDir>
#include <QDateTime>

HttpServer::HttpServer(QWidget *parent)
    : KHTTP(parent),
      m_passwdstore(nullptr)
{
    setServerID("KGet");

    m_passwdstore = new KPasswdStore(this);
    m_passwdstore->setStoreID("KGet");

    if (m_passwdstore && m_passwdstore->openStore(parent->winId())) {
        const QString usr = Settings::webinterfaceUser();
        const QString pwd = m_passwdstore->getPasswd("Webinterface", parent->winId());
        if (!setAuthenticate(usr.toUtf8(), pwd.toUtf8())) {
            KGet::showNotification(parent,
                "error", i18nc("@info", "Unable to set the WebInterface authorization: %1", errorString())
            );
            return;
        }
        if (!start(QHostAddress::Any, Settings::webinterfacePort())) {
            KGet::showNotification(parent,
                "error", i18nc("@info", "Unable to start WebInterface: %1", errorString())
            );
            return;
        }
    } else {
        KGet::showNotification(parent,
            "error", i18n("Unable to start WebInterface: Could not open KPasswdStore")
        );
    }
}

HttpServer::~HttpServer()
{
}

void HttpServer::settingsChanged()
{
    QWidget* parentwidget = qobject_cast<QWidget*>(parent());
    if (m_passwdstore && m_passwdstore->openStore(parentwidget->winId())) {
        const QString usr = Settings::webinterfaceUser();
        const QString pwd = m_passwdstore->getPasswd("Webinterface");
        stop();
        if (!setAuthenticate(usr.toUtf8(), pwd.toUtf8())) {
            KGet::showNotification(
                parentwidget,
                "error", i18nc("@info", "Unable to set the WebInterface authorization: %1", errorString())
            );
            return;
        }
        if (!start(QHostAddress::Any, Settings::webinterfacePort())) {
            KGet::showNotification(
                parentwidget,
                "error", i18nc("@info", "Unable to restart WebInterface: %1", errorString())
            );
            return;
        }
    }
}

void HttpServer::respond(const QByteArray &url, QByteArray *outdata,
                         ushort *outhttpstatus, KHTTPHeaders *outheaders,
                         QString *outfilePath)
{
    Q_UNUSED(outfilePath);
    *outhttpstatus = 200;

    QByteArray data;

    // qDebug() << Q_FUNC_INFO << url;
    if (url.endsWith("data.json")) {
        data.append("{\"downloads\":[");
        bool needsToBeClosed = false;
        foreach(TransferHandler *transfer, KGet::allTransfers()) {
            if (needsToBeClosed)
                data.append(","); // close the last line
            data.append(QString("{\"name\":\"" + transfer->source().fileName() +
                            "\", \"src\":\"" + transfer->source().prettyUrl() +
                            "\", \"dest\":\"" + transfer->dest().pathOrUrl()  +
                            "\", \"status\":\"" + transfer->statusText() +
                            "\", \"size\":\"" + KIO::convertSize(transfer->totalSize()) +
                            "\", \"progress\":\"" + QString::number(transfer->percent()) + "%"
                            "\", \"speed\":\"" + i18nc("@item speed of transfer per seconds", "%1/s",
                                                        KIO::convertSize(transfer->downloadSpeed())) + "\"}").toUtf8());
            needsToBeClosed = true;
        }
        data.append("]}");
    } else if (url.startsWith("/do")) {
        QString args = url.right(url.length() - 4);

        if (!args.isEmpty()) {
            QString action;
            QString data;
            QString group;
            QStringList argList = args.split('&');
            foreach (const QString &s, argList) {
                QStringList map = s.split('=');
                if (map.at(0) == "action")
                    action = map.at(1);
                else if (map.at(0) == "data")
                    data = KUrl::fromPercentEncoding(QByteArray(map.at(1).toUtf8()));
                // action specific parameters
                else if (map.at(0) == "group")
                    group = KUrl::fromPercentEncoding(QByteArray(map.at(1).toUtf8()));
            }
            kDebug(5001) << action << data << group;
            if (action == "add") {
                //find a folder to store the download in 
                QString defaultFolder;

                //prefer the defaultFolder of the selected group
                TransferGroupHandler *groupHandler = KGet::findGroup(group);
                if (groupHandler) {
                    defaultFolder = groupHandler->defaultFolder();
                }
                if (defaultFolder.isEmpty()) {
                    QList<TransferGroupHandler*> groups = KGet::groupsFromExceptions(KUrl(data));
                    if (groups.isEmpty() || groups.first()->defaultFolder().isEmpty()) {
                        defaultFolder = KGet::generalDestDir();
                    } else {
                        // take first item of default folder list (which should be the best one)
                        groupHandler = groups.first();
                        group = groupHandler->name();
                        defaultFolder = groupHandler->defaultFolder();
                    }
                }
                KGet::addTransfer(data, defaultFolder, KUrl(data).fileName(), group);
                data.append(QString("Ok, %1 added!").arg(data).toUtf8());
            } else if (action == "start") {
                TransferHandler *transfer = KGet::findTransfer(data);
                if (transfer)
                    transfer->start();
            } else if (action == "stop") {
                TransferHandler *transfer = KGet::findTransfer(data);
                if (transfer)
                    transfer->stop();
            } else if (action == "remove") {
                TransferHandler *transfer = KGet::findTransfer(data);
                if (transfer)
                    KGet::delTransfer(transfer);
            } else {
                kWarning(5001) << "not implemented action" << action << data;
            }
        }
    } else { // read it from filesystem
        QString fileName = QString(url).remove(".."); // disallow changing directory
        if (fileName.endsWith('/'))
            fileName = "index.htm";

        QString path = KStandardDirs::locate("data", "kget/www/" + fileName);
        QFile file(path);

        if (path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
            *outhttpstatus = 404;
            // DO NOT TRANSLATE THE FOLLOWING MESSAGE! webserver messages are never translated.
            QString notfoundText = QString("<html><head><title>404 Not Found</title></head><body>"
                                        "<h1>Not Found</h1>The requested URL <code>%1</code> "
                                        "was not found on this server.</body></html>")
                                        .arg(url.constData());
            data.append(notfoundText.toUtf8());
        } else {
            while (!file.atEnd()) {
                data.append(file.readLine());
            }
        }
        if (fileName == "index.htm") { // translations
            data.replace("#{KGet Webinterface}", i18nc("@label", "KGet Web Interface").toUtf8());
            data.replace("#{Nr}", i18nc("@label number", "Nr").toUtf8());
            data.replace("#{File name}", i18nc("@label", "File name").toUtf8());
            data.replace("#{Finished}", i18nc("@label Progress of transfer", "Finished").toUtf8());
            data.replace("#{Speed}", i18nc("@label Speed of transfer", "Speed").toUtf8());
            data.replace("#{Status}", i18nc("@label Status of transfer", "Status").toUtf8());
            data.replace("#{Start}", i18nc("@action:button start a transfer", "Start").toUtf8());
            data.replace("#{Stop}", i18nc("@action:button", "Stop").toUtf8());
            data.replace("#{Remove}", i18nc("@action:button", "Remove").toUtf8());
            data.replace("#{Source:}", i18nc("@label Download from", "Source:").toUtf8());
            data.replace("#{Saving to:}", i18nc("@label Save download to", "Saving to:").toUtf8());
            data.replace("#{Webinterface}", i18nc("@label Title in header", "Web Interface").toUtf8());
            data.replace("#{Settings}", i18nc("@action", "Settings").toUtf8());
            data.replace("#{Refresh}", i18nc("@action", "Refresh").toUtf8());
            data.replace("#{Enter URL: }", i18nc("@action", "Enter URL: ").toUtf8());
            data.replace("#{OK}", i18nc("@action:button", "OK").toUtf8());
            data.replace("#{Refresh download list every}",
                        i18nc("@action Refresh download list every x (seconds)", "Refresh download list every").toUtf8());
            data.replace("#{seconds}", i18nc("@action (Refresh very x )seconds", "seconds").toUtf8());
            data.replace("#{Save Settings}", i18nc("@action:button", "Save Settings").toUtf8());
            data.replace("#{Downloads}", i18nc("@title", "Downloads").toUtf8());
            data.replace("#{KGet Webinterface | Valid XHTML 1.0 Strict &amp; CSS}",
                        i18nc("@label text in footer", "KGet Web Interface | Valid XHTML 1.0 Strict &amp; CSS").toUtf8().replace('&', "&amp;"));

            // delegate group combobox
            QString groupOptions = "";
            Q_FOREACH(const QString &group, KGet::transferGroupNames())
                groupOptions += QString("<option>%1</option>").arg(group);
            data.replace("#{groups}", groupOptions.toUtf8());
        }
    }

    // for HTTP information see: http://www.jmarshall.com/easy/http/
    if (url.endsWith(".png") && *outhttpstatus == 200) {
        outheaders->insert("Content-Type", "image/png");
    } else if (url.endsWith(".json") && *outhttpstatus == 200) {
        outheaders->insert("Content-Type", "application/x-json");
    } else if (url.endsWith(".gif") && *outhttpstatus == 200) {
        outheaders->insert("Content-Type", "image/gif");
    } else if (url.endsWith(".js") && *outhttpstatus == 200) {
        outheaders->insert("Content-Type", "text/javascript");
    } else if (url.endsWith(".htc") && *outhttpstatus == 200) {
        outheaders->insert("Content-Type", "text/x-component");
    } else {
        outheaders->insert("Content-Type", "text/html; charset=UTF-8");
    }

    outdata->append(data);
}
