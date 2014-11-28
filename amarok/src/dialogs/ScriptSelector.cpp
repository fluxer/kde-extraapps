/****************************************************************************************
 * Copyright (c) 2008 Peter ZHOU <peterzhoulei@gmail.com>                               *
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
 
#include "ScriptSelector.h"

#include "core/support/Debug.h"

#include <KLocale>
#include <KLineEdit>

ScriptSelector::ScriptSelector( QWidget * parent )
    : KPluginSelector( parent )
    , m_scriptCount( 0 )
{
    KLineEdit* lineEdit = this->findChild<KLineEdit*>();
    if( lineEdit )
        lineEdit->setClickMessage( i18n( "Search Scripts" ) );

    m_listView = this->findChild<KCategorizedView*>();
}

ScriptSelector::~ScriptSelector()
{}

void ScriptSelector::addScripts( const QList<KPluginInfo> &pluginInfoList,
                                 PluginLoadMethod pluginLoadMethod,
                                 const QString &categoryName,
                                 const QString &categoryKey,
                                 const KSharedConfig::Ptr &config )
{
    DEBUG_BLOCK

    addPlugins( pluginInfoList, pluginLoadMethod, categoryName, categoryKey, config );
    foreach( const KPluginInfo &plugin, pluginInfoList )
    {
        m_scriptCount++;
        m_scripts[m_scriptCount] = plugin.name();
    }
}

QString ScriptSelector::currentItem() const
{
    DEBUG_BLOCK

    QItemSelectionModel *selModel = m_listView->selectionModel();
    const QModelIndexList selIndexes = selModel->selectedIndexes();

    if( !selIndexes.isEmpty() )
    {
        QModelIndex currentIndex = selIndexes[0];
        if( currentIndex.isValid() )
        {
            debug() << "row: " << currentIndex.row() + 1; //the index start from 1
            debug() << "name: "<< m_scripts[currentIndex.row() + 1];
            return m_scripts[currentIndex.row() + 1];
        }
    }

    return QString();
}

#include "ScriptSelector.moc"
