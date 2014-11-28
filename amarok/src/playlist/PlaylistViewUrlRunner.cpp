/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2009 Téo Mrnjavac <teo@kde.org>                                        *
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

#include "PlaylistViewUrlRunner.h"

#include "MainWindow.h"
#include "amarokurls/AmarokUrlHandler.h"
#include "core/support/Debug.h"
#include "layouts/LayoutManager.h"
#include "playlist/PlaylistDock.h"
#include "playlist/ProgressiveSearchWidget.h"

#include <KStandardDirs>
#include <KLocale>

#include <QList>
#include <QStringList>
#include <QActionGroup>

namespace Playlist
{

ViewUrlRunner::ViewUrlRunner()
    : AmarokUrlRunnerBase()
{}

ViewUrlRunner::~ViewUrlRunner()
{
    The::amarokUrlHandler()->unRegisterRunner( this );
}

bool
ViewUrlRunner::run( AmarokUrl url )
{
    DEBUG_BLOCK

    QMap< QString, QString > args = url.args();
    QWeakPointer<Dock> playlistDock = The::mainWindow()->playlistDock();

    if( args.keys().contains( "filter" ) )
    {
        QString filterExpr = args.value( "filter" );
        playlistDock.data()->searchWidget()->setCurrentFilter( filterExpr );
        if( args.keys().contains( "matches" ) )
        {
            QString onlyMatches = args.value( "matches" );
            playlistDock.data()->searchWidget()->slotShowOnlyMatches( ( onlyMatches == QString( "true" ) ) );
        }
    }
    if( args.keys().contains( "sort" ) )
    {
        playlistDock.data()->sortWidget()->trimToLevel();

        QString sortPath = args.value( "sort" );

        QStringList levels = sortPath.split( '-' );
        foreach( const QString &level, levels )
        {
            if( level == QString( "Random" ) || level == QString( "Shuffle" ) ) //we keep "Random" for compatibility
            {
                playlistDock.data()->sortWidget()->addLevel( QString( "Shuffle" ) );
                break;
            }
            QStringList levelParts = level.split( '_' );
            if( levelParts.count() > 2 )
                warning() << "Playlist view URL parse error: Invalid sort level " << level;
            if( levelParts.at( 1 ) == QString( "asc" ) )
                playlistDock.data()->sortWidget()->addLevel( levelParts.at( 0 ), Qt::AscendingOrder );
            else if( levelParts.at( 1 ) == QString( "des" ) )
                playlistDock.data()->sortWidget()->addLevel( levelParts.at( 0 ), Qt::DescendingOrder );
            else
                warning() << "Playlist view URL parse error: Invalid sort order for level " << level;
        }
    }

    if( args.keys().contains( "layout" ) )
    {
        QString layout = args.value( "layout" );
        LayoutManager::instance()->setActiveLayout( layout );
    }
    
    The::mainWindow()->showDock( MainWindow::AmarokDockPlaylist );

    return true;
}

QString
ViewUrlRunner::command() const
{
    return "playlist";
}

QString
ViewUrlRunner::prettyCommand() const
{
    return i18nc( "A type of command that affects the sorting, layout and filtering int he Playlist", "Playlist" );
}

KIcon
ViewUrlRunner::icon() const
{
    return KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/playlist-bookmark-16.png" ) ) );
}

} //namespace Playlist
