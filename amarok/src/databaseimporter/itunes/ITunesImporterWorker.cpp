/****************************************************************************************
 * Copyright (c) 2008 Seb Ruiz <ruiz@kde.org>                                           *
 * Copyright (c) 2008 Leo Franchi <lfranchi@kde.org>                                    *
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

#include "ITunesImporterWorker.h"

#include "core/support/Amarok.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "core/collections/CollectionLocation.h"
#include "core/support/Debug.h"
#include "core-impl/collections/support/FileCollectionLocation.h"
#include "core-impl/meta/file/File.h"

#include <kio/job.h>
#include <kio/jobclasses.h>

#include <QFile>
#include <QDateTime>

ITunesImporterWorker::ITunesImporterWorker()
    : ThreadWeaver::Job()
    , m_aborted( false )
    , m_failed( false )
{
}

ITunesImporterWorker::~ITunesImporterWorker()
{
}

void
ITunesImporterWorker::readTrackElement()
{
    QString title, artist, album, url;
    int year = -1, bpm = -1, playcount = -1, rating = -1;
    QDateTime lastplayed;

    while( !( isEndElement() && name() == "dict" ) )
    {
        readNext();
        QString text = readElementText();
        if( name() == "key" && text == "Name" )
        {
            readNext(); // skip past the </key> and to the data tag
            QString text = readElementText();
            title = text;
        } else if( name() == "key" && text == "Artist" )
        {
            readNext(); // skip past the </key> and to the data tag
            artist = readElementText();
        } else if( isStartElement() && name() == "key" && text == "Album" )
        {
            readNext(); // skip past the </key> and to the data tag
            album = readElementText();
        } else if( name() == "key" && text == "Year" )
        {
            readNext(); // skip past the </key> and to the data tag
            year = readElementText().toInt();
        } else if( name() == "key" && text == "BPM" )
        {
            readNext(); // skip past the </key> and to the data tag
            bpm = readElementText().toInt();
        } else if( name() == "key" && text == "Play Count" )
        {
            readNext(); // skip past the </key> and to the data tag
            playcount = readElementText().toInt();
        } else if( name() == "key" && text == "Rating" )
          {
            readNext(); // skip past the </key> and to the data tag
            rating = readElementText().toInt() / 10; // itunes rates 0-100
        } else if( name() == "key" && text == "Play Date" )
        {
            readNext(); // skip past the </key> and to the data tag
            lastplayed = QDateTime::fromTime_t(readElementText().toInt());
        } else if( name() == "key" && text == "Location" )
        {
            readNext(); // skip past the </key> and to the data tag
            url = readElementText();
        }
    }

    //split the file://localhost/path/to/track   to just file:///path/to/track
    if( url.indexOf( "file://localhost" ) == 0 )
        url = url.remove( 7, 9 );

    debug() << "got track info:" << title << artist << album << year << bpm << url;

    Meta::TrackPtr track = CollectionManager::instance()->trackForUrl( KUrl( url ) );
    if( track )
    {
        Meta::StatisticsPtr statistics = track->statistics();
        statistics->beginUpdate();
        if( rating != -1 )
            statistics->setRating( rating );
        if( lastplayed.isValid() )
            statistics->setLastPlayed( lastplayed );
        if( playcount != -1 )
            statistics->setPlayCount( playcount );
        statistics->endUpdate();

        if( !track->inCollection() )
        {
            m_tracksForInsert.insert( track, track->playableUrl().url() );
            debug() << " inserting track:" << track->playableUrl();
        }
        else {
            Collections::Collection* collection = track->collection();
            if (collection)
                debug() << "track in collection (" << collection->location()->prettyLocation() << "):" << track->playableUrl();
        }

        emit trackAdded( track );
    }
}

void
ITunesImporterWorker::run()
{
    DEBUG_BLOCK
    debug() << "file:" << m_databaseLocation;
    QScopedPointer<QFile> file( new QFile( m_databaseLocation ) );
    if( !file->exists() )
    {
        debug() << "COULDN'T FIND DB FILE!";
        emit importError( "" );
        m_failed = true;
        return;
    }
    if ( !file->open( QIODevice::ReadOnly ) )
    {
        debug() << "COULDN'T OPEN DB FILE!";
        emit importError( "" );
        m_failed = true;
        return;
    }
    setDevice( file.data() );

    //debug() << "got element:" << name().toString();

    while( !atEnd() )
    {
        if( m_aborted )
            return;

        readNext();

        if ( name() == "key" && readElementText() == "Tracks" ) // ok, we're at the start
        {
            readNext();
            readNext();
            readNext(); // this skips the first all-encompassing <dict> tag
            debug() << "got start of tracks";
            while( !atEnd() && !( isEndElement() && name() == "dict" ) )
            {
                if( m_aborted )
                    return;

                //debug() << "reading element name:" << name().toString();
                if( isStartElement() && name() == "dict") // this is a track item!
                {
                    readTrackElement();
                }
                readNext();
            }
        }
    }

    if( m_tracksForInsert.size() > 0 )
    {
        Collections::CollectionLocation *location = CollectionManager::instance()->primaryCollection()->location();

        QMapIterator<Meta::TrackPtr, QString> j(m_tracksForInsert);
        while (j.hasNext()) {
            j.next();
            location->insert( j.key(), j.value() );
        }
    }

    debug() << "done importing xml file";
}


#include "ITunesImporterWorker.moc"

