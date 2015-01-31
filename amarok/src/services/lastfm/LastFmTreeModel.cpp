/****************************************************************************************
 * Copyright (c) 2008 Casey Link <unnamedrambler@gmail.com>                             *
 * Copyright (c) 2009 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2009 Mark Kretschmann <kretschmann@kde.org>                            *
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

#define DEBUG_PREFIX "LastFmTreeModel"
#include "core/support/Debug.h"

#include "LastFmTreeModel.h"

#include "AvatarDownloader.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "AmarokMimeData.h"

#include <KIcon>

#include <QPainter>

#include <Tag.h>
#include <XmlQuery.h>

using namespace LastFm;

LastFmTreeModel::LastFmTreeModel( QObject *parent )
    : QAbstractItemModel( parent )
{
    m_rootItem = new LastFmTreeItem( LastFm::Root, "Hello" );
    setupModelData( m_rootItem );

    QNetworkReply *reply;
    reply = m_user.getNeighbours();
    connect(reply, SIGNAL(finished()), this, SLOT(slotAddNeighbors()) );

    reply = m_user.getFriends();
    connect( reply, SIGNAL(finished()), this, SLOT(slotAddFriends()) );

    reply = m_user.getTopTags();
    connect( reply, SIGNAL(finished()), this, SLOT(slotAddTags()) );

    reply = m_user.getTopArtists();
    connect( reply, SIGNAL(finished()), this, SLOT(slotAddTopArtists()) );
}

LastFmTreeModel::~LastFmTreeModel()
{
    delete m_rootItem;
}

void
LastFmTreeModel::slotAddNeighbors()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
    if( !reply )
    {
        debug() << __PRETTY_FUNCTION__ << "null reply!";
        return;
    }
    reply->deleteLater();

    lastfm::XmlQuery lfm;
    if( lfm.parse( reply->readAll() ) )
    {
        QList<lastfm::XmlQuery> children = lfm[ "neighbours" ].children( "user" );
        int start = m_myNeighbors->childCount();
        QModelIndex parent = index( m_myNeighbors->row(), 0 );
        beginInsertRows( parent, start, start + children.size() );

        foreach( const lastfm::XmlQuery &e, children  )
        {
            const QString name = e[ "name" ].text();

            LastFmTreeItem* neighbor = new LastFmTreeItem( mapTypeToUrl(LastFm::NeighborsChild, name),
                                                           LastFm::NeighborsChild, name, m_myNeighbors );
            KUrl avatarUrl( e[ QLatin1String("image size=small") ].text() );
            if( !avatarUrl.isEmpty() )
                neighbor->setAvatarUrl( avatarUrl );

            m_myNeighbors->appendChild( neighbor );
            appendUserStations( neighbor, name );
        }

        endInsertRows();
        emit dataChanged( parent, parent );
    }
    else
    {
        debug() << "Got exception in parsing from last.fm:" << lfm.parseError().message();
        return;
    }
}

void
LastFmTreeModel::slotAddFriends()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
    if( !reply )
    {
        debug() << __PRETTY_FUNCTION__ << "null reply!";
        return;
    }
    reply->deleteLater();

    lastfm::XmlQuery lfm;
    if( lfm.parse( reply->readAll() ) )
    {
        QList<lastfm::XmlQuery> children = lfm[ "friends" ].children( "user" );
        int start = m_myFriends->childCount();
        QModelIndex parent = index( m_myFriends->row(), 0 );
        beginInsertRows( parent, start, start + children.size() );

        foreach( const lastfm::XmlQuery &e, children )
        {
            const QString name = e[ "name" ].text();

            LastFmTreeItem* afriend = new LastFmTreeItem( mapTypeToUrl(LastFm::FriendsChild, name),
                                                          LastFm::FriendsChild, name, m_myFriends );

            KUrl avatarUrl( e[ QLatin1String("image size=small") ].text() );
            if( !avatarUrl.isEmpty() )
                afriend->setAvatarUrl( avatarUrl );

            m_myFriends->appendChild( afriend );
            appendUserStations( afriend, name );
        }

        endInsertRows();
        emit dataChanged( parent, parent );
    }
    else
    {
        debug() << "Got exception in parsing from last.fm:" << lfm.parseError().message();
        return;
    }
}

void
LastFmTreeModel::slotAddTags()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
    if( !reply )
    {
        debug() << __PRETTY_FUNCTION__ << "null reply!";
        return;
    }
    reply->deleteLater();

    QMap<int, QString> listWithWeights = lastfm::Tag::list( reply );
    int start = m_myTags->childCount();
    QModelIndex parent = index( m_myTags->row(), 0 );
    beginInsertRows( parent, start, start + listWithWeights.size() );

    QMapIterator<int, QString> it( listWithWeights );
    it.toBack();
    while( it.hasPrevious() )
    {
        it.previous();
        int count = it.key();
        QString text = it.value();
        QString prettyText = i18nc( "%1 is Last.fm tag name, %2 is its usage count",
                                    "%1 (%2)", text, count );

        LastFmTreeItem *tag = new LastFmTreeItem( mapTypeToUrl( LastFm::MyTagsChild, text ),
                                                  LastFm::MyTagsChild, prettyText, m_myTags );
        m_myTags->appendChild( tag );
    }

    endInsertRows();
    emit dataChanged( parent, parent );
}

void
LastFmTreeModel::slotAddTopArtists()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
    if( !reply )
    {
        debug() << __PRETTY_FUNCTION__ << "null reply!";
        return;
    }
    reply->deleteLater();

    QMultiMap<int, QString> playcountArtists;
    lastfm::XmlQuery lfm;
    if( lfm.parse( reply->readAll() ) )
    {
        foreach( const lastfm::XmlQuery &e, lfm[ "topartists" ].children( "artist" ) )
        {
            QString name = e[ "name" ].text();
            int playcount = e[ "playcount" ].text().toInt();
            playcountArtists.insert( playcount, name );
        }
    }
    else
    {
        debug() << "Got exception in parsing from last.fm:" << lfm.parseError().message();
        return;
    }

    int start = m_myTopArtists->childCount();
    QModelIndex parent = index( m_myTopArtists->row(), 0 );
    beginInsertRows( parent, start, start + playcountArtists.size() );

    QMapIterator<int, QString> it( playcountArtists );
    it.toBack();
    while( it.hasPrevious() )
    {
        it.previous();
        int count = it.key();
        QString text = it.value();
        QString prettyText = i18ncp( "%2 is artist name, %1 is number of plays",
                                     "%2 (%1 play)", "%2 (%1 plays)", count, text );

        LastFmTreeItem *artist = new LastFmTreeItem( mapTypeToUrl( LastFm::ArtistsChild, text ),
                                                     LastFm::ArtistsChild, prettyText, m_myTopArtists );
        m_myTopArtists->appendChild( artist );
    }

    endInsertRows();
    emit dataChanged( parent, parent );
}

void
LastFmTreeModel::appendUserStations( LastFmTreeItem* item, const QString &user )
{
    // no need to call begin/endInsertRows() or dataChanged(), we're being called inside
    // beginInsertRows().
    LastFmTreeItem* personal = new LastFmTreeItem( mapTypeToUrl( LastFm::UserChildPersonal, user ),
                                                   LastFm::UserChildPersonal, i18n( "Personal Radio" ), item );
    LastFmTreeItem* neigh = new LastFmTreeItem( mapTypeToUrl( LastFm::UserChildNeighborhood, user ),
                                                LastFm::UserChildNeighborhood, i18n( "Neighborhood" ), item );
    item->appendChild( personal );
    item->appendChild( neigh );
}

void
LastFmTreeModel::prepareAvatar( QPixmap &avatar, int size )
{
    // This code is here to stop Qt from crashing on certain weirdly shaped avatars.
    // We had a case were an avatar got a height of 1px after scaling and it would
    // crash in the rendering code. This here just fills in the background with
    // transparency first.
    if( avatar.width() < size || avatar.height() < size )
    {
        QImage finalAvatar( size, size, QImage::Format_ARGB32 );
        finalAvatar.fill( 0 );

        QPainter p( &finalAvatar );
        QRect r;

        if( avatar.width() < size )
            r = QRect( ( size - avatar.width() ) / 2, 0, avatar.width(), avatar.height() );
        else
            r = QRect( 0, ( size - avatar.height() ) / 2, avatar.width(), avatar.height() );

        p.drawPixmap( r, avatar );
        p.end();

        avatar = QPixmap::fromImage( finalAvatar );
    }
}

void
LastFmTreeModel::onAvatarDownloaded( const QString &username, QPixmap avatar )
{
    sender()->deleteLater();
    if( avatar.isNull() || avatar.height() <= 0 || avatar.width() <= 0 )
        return;
    if( username == m_user.name() )
        return;

    int m = avatarSize();
    avatar = avatar.scaled( m, m, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    prepareAvatar( avatar, m );
    m_avatars.insert( username, avatar );

    // these 2 categories have a chance to be updated:
    QList<LastFmTreeItem *> categories;
    categories << m_myFriends << m_myNeighbors;

    // now go through all children of the categories and notify view as appropriate
    foreach( LastFmTreeItem *category, categories )
    {
        QModelIndex parentIdx = index( category->row(), 0 );
        for( int i = 0; i < category->childCount(); i++ )
        {
            LastFmTreeItem *item = category->child( i );
            if( !item )
                continue;

            if( item->data() == username )
            {
                QModelIndex idx = index( i, 0, parentIdx );
                emit dataChanged( idx, idx );
                break; // no user is twice in a single category
            }
        }
    }
}

QIcon
LastFmTreeModel::avatar( const QString &username, const KUrl &avatarUrl ) const
{
    KIcon defaultIcon( "filename-artist-amarok" );
    if( username.isEmpty() )
        return defaultIcon;
    if( m_avatars.contains(username) )
        return m_avatars.value( username );
    if( !avatarUrl.isValid() )
        return defaultIcon;

     // insert placeholder so that we don't request the save avatar twice;
    const_cast<LastFmTreeModel *>( this )->m_avatars.insert( username, defaultIcon );
    AvatarDownloader* downloader = new AvatarDownloader();
    downloader->downloadAvatar( username, avatarUrl );
    connect( downloader, SIGNAL(avatarDownloaded(QString,QPixmap)),
                         SLOT(onAvatarDownloaded(QString,QPixmap)) );
    return defaultIcon;
}

int
LastFmTreeModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent )
    return 1;
}

int
LastFmTreeModel::avatarSize()
{
    return 32;
}

QVariant
LastFmTreeModel::data( const QModelIndex &index, int role ) const
{
    if( !index.isValid() )
        return QVariant();

    LastFmTreeItem *i = static_cast<LastFmTreeItem*>( index.internalPointer() );
    if( role == Qt::DisplayRole )
        switch( i->type() )
        {
        case MyRecommendations:
            return i18n( "My Recommendations" );
        case PersonalRadio:
            return i18n( "My Radio Station" );
        case MixRadio:
            return i18n( "My Mix Radio" );
        case NeighborhoodRadio:
            return i18n( "My Neighborhood" );
            //             case RecentlyPlayed:      return tr("Recently Played");
            //             case RecentlyLoved:       return tr("Recently Loved");
            //             case RecentlyBanned:      return tr("Recently Banned");
        case TopArtists:
            return i18n( "My Top Artists" );
        case MyTags:
            return i18n( "My Tags" );
        case Friends:
            return i18n( "Friends" );
        case Neighbors:
            return i18n( "Neighbors" );
            //             case History:             return tr("History");

            //             case RecentlyPlayedTrack: return m_played.value( index.row() );
            //             case RecentlyLovedTrack:  return m_loved.value( index.row() );
            //             case RecentlyBannedTrack: return m_banned.value( index.row() );
//             case MyTagsChild:         return m_tags.value( index.row() );
        case FriendsChild:
        case ArtistsChild:
        case NeighborsChild:
        case UserChildPersonal:
        case UserChildNeighborhood:
        case MyTagsChild:
            return i->data();
        default:
            break;
        }

    if( role == Qt::DecorationRole )
        switch( i->type() )
        {
        case MyRecommendations:
            return KIcon( "lastfm-recommended-radio-amarok" );
        case TopArtists:
        case PersonalRadio:
            return KIcon( "lastfm-personal-radio-amarok" );
        case MixRadio:
            return KIcon( "lastfm-mix-radio-amarok" );
        case NeighborhoodRadio:
            return KIcon( "lastfm-neighbour-radio-amarok" );
            //             case RecentlyPlayed:      return KIcon( "lastfm-recent-tracks-amarok" );
            //             case RecentlyLoved:       return KIcon( "lastfm-recently-loved-amarok" );
            //             case RecentlyBanned:      return KIcon( "lastfm-recently-banned-amarok" );
        case MyTags:
            return KIcon( "lastfm-my-tags-amarok" );
        case Friends:
            return KIcon( "lastfm-my-friends-amarok" );
        case Neighbors:
            return KIcon( "lastfm-my-neighbours-amarok" );

        case RecentlyPlayedTrack: //FALL THROUGH
        case RecentlyLovedTrack:  //FALL THROUGH
        case RecentlyBannedTrack:
            return KIcon( "icon_track" );
        case MyTagsChild:
            return KIcon( "lastfm-tag-amarok" );

        case FriendsChild:
            return avatar( i->data().toString(), i->avatarUrl() );
        case UserChildPersonal:
            return KIcon( "lastfm-personal-radio-amarok" );
        case UserChildNeighborhood:
            return KIcon( "lastfm-neighbour-radio-amarok" );

        case NeighborsChild:
            return avatar( i->data().toString(), i->avatarUrl() );

        case HistoryStation:
            return KIcon( "icon_radio" );

        default:
            break;
        }

        if( role == LastFm::TrackRole )
        {
            switch( i->type() )
            {
                case LastFm::MyRecommendations:
                case LastFm::PersonalRadio:
                case LastFm::MixRadio:
                case LastFm::NeighborhoodRadio:
                case LastFm::FriendsChild:
                case LastFm::NeighborsChild:
                case LastFm::MyTagsChild:
                case LastFm::ArtistsChild:
                case LastFm::UserChildPersonal:
                case LastFm::UserChildNeighborhood:
                    return QVariant::fromValue( i->track() );
                default:
                    break;
            }
        }
        if( role == LastFm::TypeRole )
            return i->type();

    return QVariant();
}

Qt::ItemFlags
LastFmTreeModel::flags( const QModelIndex &index ) const
{
    if( !index.isValid() )
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    LastFmTreeItem *i = static_cast<LastFmTreeItem*>( index.internalPointer() );
    switch( i->type() )
    {
    case MyRecommendations:
    case PersonalRadio:
    case MixRadio:
    case NeighborhoodRadio:
    case RecentlyPlayedTrack:
    case RecentlyLovedTrack:
    case RecentlyBannedTrack:
    case MyTagsChild:
    case FriendsChild:
    case ArtistsChild:
    case NeighborsChild:
    case HistoryStation:
    case UserChildPersonal:
    case UserChildNeighborhood:
        flags |= Qt::ItemIsSelectable;
        break;

    default:
        break;
    }

    switch( i->type() )
    {
    case UserChildPersonal:
    case UserChildNeighborhood:
    case MyTagsChild:
    case ArtistsChild:
    case MyRecommendations:
    case PersonalRadio:
    case MixRadio:
    case NeighborhoodRadio:
        flags |= Qt::ItemIsDragEnabled;

    default:
        break;
    }

    return flags;
}

QModelIndex
LastFmTreeModel::index( int row, int column, const QModelIndex &parent )
const
{
    if( !hasIndex( row, column, parent ) )
        return QModelIndex();

    LastFmTreeItem *parentItem;

    if( !parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = static_cast<LastFmTreeItem*>( parent.internalPointer() );

    LastFmTreeItem *childItem = parentItem->child( row );
    if( childItem )
        return createIndex( row, column, childItem );
    else
        return QModelIndex();
}

QModelIndex
LastFmTreeModel::parent( const QModelIndex &index ) const
{
    if( !index.isValid() )
        return QModelIndex();

    LastFmTreeItem *childItem = static_cast<LastFmTreeItem*>( index.internalPointer() );
    LastFmTreeItem *parentItem = childItem->parent();

    if( parentItem == m_rootItem )
        return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}

int
LastFmTreeModel::rowCount( const QModelIndex &parent ) const
{
    LastFmTreeItem *parentItem;
    if( parent.column() > 0 )
        return 0;

    if( !parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = static_cast<LastFmTreeItem*>( parent.internalPointer() );

    return parentItem->childCount();
}

void
LastFmTreeModel::setupModelData( LastFmTreeItem *parent )
{
    // no need to call beginInsertRows() here, this is only called from constructor
    parent->appendChild( new LastFmTreeItem( mapTypeToUrl( LastFm::MyRecommendations ), LastFm::MyRecommendations, parent ) );
    parent->appendChild( new LastFmTreeItem( mapTypeToUrl( LastFm::PersonalRadio ), LastFm::PersonalRadio, parent ) );
    parent->appendChild( new LastFmTreeItem( mapTypeToUrl( LastFm::MixRadio ), LastFm::MixRadio, parent ) );
    parent->appendChild( new LastFmTreeItem( mapTypeToUrl( LastFm::NeighborhoodRadio ), LastFm::NeighborhoodRadio, parent ) );

    m_myTopArtists = new LastFmTreeItem( LastFm::TopArtists, parent );
    parent->appendChild( m_myTopArtists );

    m_myTags = new LastFmTreeItem( LastFm::MyTags, parent );
    parent->appendChild( m_myTags );

    m_myFriends = new LastFmTreeItem( LastFm::Friends, parent );
    parent->appendChild( m_myFriends );

    m_myNeighbors = new LastFmTreeItem( LastFm::Neighbors, parent );
    parent->appendChild( m_myNeighbors );
}

QString
LastFmTreeModel::mapTypeToUrl( LastFm::Type type, const QString &key )
{
    QString const encoded_username = KUrl::toPercentEncoding( m_user.name() );
    switch( type )
    {
    case MyRecommendations:
        return "lastfm://user/" + encoded_username + "/recommended";
    case PersonalRadio:
        return "lastfm://user/" + encoded_username + "/personal";
    case MixRadio:
        return "lastfm://user/" + encoded_username + "/mix";
    case NeighborhoodRadio:
        return "lastfm://user/" + encoded_username + "/neighbours";
    case MyTagsChild:
        return "lastfm://usertags/" + encoded_username + "/" + KUrl::toPercentEncoding( key );
    case FriendsChild:
        return "lastfm://user/" + KUrl::toPercentEncoding( key ) + "/personal";
    case ArtistsChild:
        return "lastfm://artist/" + KUrl::toPercentEncoding( key ) + "/similarartists";
    case NeighborsChild:
        return "lastfm://user/" + KUrl::toPercentEncoding( key ) + "/personal";
    case UserChildPersonal:
        return "lastfm://user/" + KUrl::toPercentEncoding( key ) + "/personal";
    case UserChildNeighborhood:
        return "lastfm://user/" + KUrl::toPercentEncoding( key ) + "/neighbours";
    default:
        return "";
    }
}

LastFmTreeItem::LastFmTreeItem( const LastFm::Type &type, const QVariant &data, LastFmTreeItem *parent )
        : mType( type ), parentItem( parent ), itemData( data )
{
}

LastFmTreeItem::LastFmTreeItem( const LastFm::Type &type, LastFmTreeItem *parent )
        : mType( type ), parentItem( parent )
{

}

LastFmTreeItem::LastFmTreeItem( const QString &url, const LastFm::Type &type, LastFmTreeItem *parent )
        : mType( type ), parentItem( parent ), mUrl( url )
{

}

LastFmTreeItem::LastFmTreeItem( const QString &url, const LastFm::Type &type, const QVariant &data, LastFmTreeItem *parent )
        : mType( type ), parentItem( parent ), itemData( data ), mUrl( url )
{
}

LastFmTreeItem::~LastFmTreeItem()
{
    qDeleteAll( childItems );
}

void
LastFmTreeItem::appendChild( LastFmTreeItem *item )
{
    childItems.append( item );
}

LastFmTreeItem *
LastFmTreeItem::child( int row )
{
    return childItems.value( row );
}

int
LastFmTreeItem::childCount() const
{
    return childItems.count();
}

QVariant
LastFmTreeItem::data() const
{
    return itemData;
}

Meta::TrackPtr
LastFmTreeItem::track() const
{
    Meta::TrackPtr track;
    if( mUrl.isEmpty() )
        return track;

    KUrl url( mUrl );
    track = CollectionManager::instance()->trackForUrl( url );

    return track;
}

LastFmTreeItem *LastFmTreeItem::parent()
{
    return parentItem;
}

int
LastFmTreeItem::row() const
{
    if( parentItem )
        return parentItem->childItems.indexOf( const_cast<LastFmTreeItem*>( this ) );

    return 0;
}

QMimeData*
LastFmTreeModel::mimeData( const QModelIndexList &indices ) const
{
    debug() << "LASTFM drag items : " << indices.size();
    Meta::TrackList list;
    foreach( const QModelIndex &item, indices )
    {
        Meta::TrackPtr track = data( item, LastFm::TrackRole ).value< Meta::TrackPtr >();
        if( track )
            list << track;
    }
    qStableSort( list.begin(), list.end(), Meta::Track::lessThan );

    AmarokMimeData *mimeData = new AmarokMimeData();
    mimeData->setTracks( list );
    return mimeData;
}