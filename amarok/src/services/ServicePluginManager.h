/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/
 
#ifndef AMAROK_SERVICEPLUGINMANAGER_H
#define AMAROK_SERVICEPLUGINMANAGER_H

#include <QObject>
#include <QStringList>
#include <QSet>

class ServiceBase;
class ServiceBrowser;
class ServiceFactory;
namespace Plugins {
    class PluginFactory;
}

/**
A class to keep track of available service plugins and load them as needed

    @author
*/
class ServicePluginManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY( ServiceBrowser* browser READ browser WRITE setBrowser )

public:
    ServicePluginManager( QObject *parent = 0 );
    ~ServicePluginManager();

    ServiceBrowser *browser() const;
    void setBrowser( ServiceBrowser * browser );

    /**
     * Load any services that are configured to be loaded
     */
    void init( const QList<Plugins::PluginFactory*> &factories );

    void checkEnabledStates( const QList<Plugins::PluginFactory*> &factories );

public slots:
    QStringList loadedServices() const;
    QStringList loadedServiceNames() const;
    QString serviceDescription( const QString &service );
    QString serviceMessages( const QString &service );
    QString sendMessage( const QString &service, const QString &message );

private:
    void initFactory( ServiceFactory *factory );

    ServiceBrowser *m_serviceBrowser;

private slots:
    void slotNewService( ServiceBase *newService);
    void slotRemoveService( ServiceBase *removedService );
};


#endif //AMAROK_SERVICEPLUGINMANAGER_H

