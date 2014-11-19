/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "annotationproxymodels.h"

#include <QtCore/QList>
#include <QtGui/QItemSelection>

#include <kicon.h>

#include "annotationmodel.h"

static quint32 mixIndex( int row, int column )
{
    return ( row << 4 ) | column;
}

PageFilterProxyModel::PageFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent ),
    mGroupByCurrentPage( false ),
    mCurrentPage( -1 )
{
  setDynamicSortFilter( true );
}

void PageFilterProxyModel::groupByCurrentPage( bool value )
{
  if ( mGroupByCurrentPage == value )
    return;

  mGroupByCurrentPage = value;

  invalidateFilter();
}

void PageFilterProxyModel::setCurrentPage( int page )
{
  if ( mCurrentPage == page )
    return;

  mCurrentPage = page;

  // no need to invalidate when we're not showing the current page only
  if ( !mGroupByCurrentPage )
    return;

  invalidateFilter();
}

bool PageFilterProxyModel::filterAcceptsRow( int row, const QModelIndex &sourceParent ) const
{
  if ( !mGroupByCurrentPage )
    return true;

  const QModelIndex pageIndex = sourceModel()->index( row, 0, sourceParent );
  int page = sourceModel()->data( pageIndex, AnnotationModel::PageRole ).toInt();

  return (page == mCurrentPage);
}


PageGroupProxyModel::PageGroupProxyModel( QObject *parent )
  : QAbstractProxyModel( parent ),
    mGroupByPage( false )
{
}

int PageGroupProxyModel::columnCount( const QModelIndex &parentIndex ) const
{
  // For top-level and second level we have always only one column
  if ( mGroupByPage ) {
    if ( parentIndex.isValid() ) {
      if ( parentIndex.parent().isValid() )
        return 0;
      else {
        return 1; // second-level
      }
    } else {
      return 1; // top-level
    }
  } else {
    if ( !parentIndex.isValid() ) // top-level
      return 1;
    else
      return 0;
  }
  return 1;
}

int PageGroupProxyModel::rowCount( const QModelIndex &parentIndex ) const
{
  if ( mGroupByPage ) {
    if ( parentIndex.isValid() ) {
      if ( parentIndex.parent().isValid() )
        return 0;
      else {
        return mTreeIndexes[ parentIndex.row() ].second.count(); // second-level
      }
    } else {
      return mTreeIndexes.count(); // top-level
    }
  } else {
    if ( !parentIndex.isValid() ) // top-level
      return mIndexes.count();
    else
      return 0;
  }
}

QModelIndex PageGroupProxyModel::index( int row, int column, const QModelIndex &parentIndex ) const
{
  if ( row < 0 || column != 0 )
    return QModelIndex();

  if ( mGroupByPage ) {
    if ( parentIndex.isValid() ) {
      if ( parentIndex.row() >= 0 && parentIndex.row() < mTreeIndexes.count()
           && row < mTreeIndexes[ parentIndex.row() ].second.count() )
        return createIndex( row, column, qint32( parentIndex.row() + 1 ) );
      else
        return QModelIndex();
    } else {
      if ( row < mTreeIndexes.count() )
        return createIndex( row, column, 0 );
      else
        return QModelIndex();
    }
  } else {
    if ( row < mIndexes.count() )
      return createIndex( row, column, mixIndex( parentIndex.row(), parentIndex.column() ) );
    else
      return QModelIndex();
  }
}

QModelIndex PageGroupProxyModel::parent( const QModelIndex &idx ) const
{
  if ( mGroupByPage ) {
    if ( idx.internalId() == 0 ) // top-level
      return QModelIndex();
    else
      return index( idx.internalId() - 1, idx.column() );
  } else {
    // We have only top-level items
    return QModelIndex();
  }
}

QModelIndex PageGroupProxyModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
  if ( mGroupByPage ) {
    if ( sourceIndex.parent().isValid() ) {
      return index( sourceIndex.row(), sourceIndex.column(), sourceIndex.parent() );
    } else {
      return index( sourceIndex.row(), sourceIndex.column() );
    }
  } else {
    for ( int i = 0; i < mIndexes.count(); ++i ) {
      if ( mIndexes[ i ] == sourceIndex )
        return index( i, 0 );
    }

    return QModelIndex();
  }
}

QModelIndex PageGroupProxyModel::mapToSource( const QModelIndex &proxyIndex ) const
{
  if ( !proxyIndex.isValid() )
    return QModelIndex();

  if ( mGroupByPage ) {
    if ( proxyIndex.internalId() == 0 ) {

      if ( proxyIndex.row() >= mTreeIndexes.count() || proxyIndex.row() < 0 )
        return QModelIndex();

      return mTreeIndexes[ proxyIndex.row() ].first;
    } else {
      if ( proxyIndex.internalId() - 1 >= mTreeIndexes.count() ||
           proxyIndex.row() >= mTreeIndexes[ proxyIndex.internalId() - 1 ].second.count() )
        return QModelIndex();

      return mTreeIndexes[ proxyIndex.internalId() - 1 ].second[ proxyIndex.row() ];
    }
  } else {
    if ( proxyIndex.column() > 0 || proxyIndex.row() >= mIndexes.count() )
      return QModelIndex();
    else {
      return mIndexes[ proxyIndex.row() ];
    }
  }
}

void PageGroupProxyModel::setSourceModel( QAbstractItemModel *model )
{
  if ( sourceModel() ) {
    disconnect( sourceModel(), SIGNAL(layoutChanged()), this, SLOT(rebuildIndexes()) );
    disconnect( sourceModel(), SIGNAL(modelReset()), this, SLOT(rebuildIndexes()) );
    disconnect( sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rebuildIndexes()) );
    disconnect( sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rebuildIndexes()) );
  }

  QAbstractProxyModel::setSourceModel( model );

  connect( sourceModel(), SIGNAL(layoutChanged()), this, SLOT(rebuildIndexes()) );
  connect( sourceModel(), SIGNAL(modelReset()), this, SLOT(rebuildIndexes()) );
  connect( sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rebuildIndexes()) );
  connect( sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rebuildIndexes()) );

  rebuildIndexes();
}

void PageGroupProxyModel::rebuildIndexes()
{
  if ( mGroupByPage ) {
    mTreeIndexes.clear();

    for ( int row = 0; row < sourceModel()->rowCount(); ++row ) {
      const QModelIndex pageIndex = sourceModel()->index( row, 0 );

      QList<QModelIndex> itemIndexes;
      for ( int subRow = 0; subRow < sourceModel()->rowCount( pageIndex ); ++subRow ) {
        itemIndexes.append( sourceModel()->index( subRow, 0, pageIndex ) );
      }

      mTreeIndexes.append( QPair<QModelIndex, QList<QModelIndex> >( pageIndex, itemIndexes ) );
    }
  } else {
    mIndexes.clear();

    for ( int row = 0; row < sourceModel()->rowCount(); ++row ) {
      const QModelIndex pageIndex = sourceModel()->index( row, 0 );
      for ( int subRow = 0; subRow < sourceModel()->rowCount( pageIndex ); ++subRow ) {
        mIndexes.append( sourceModel()->index( subRow, 0, pageIndex ) );
      }
    }
  }

  reset();
}

void PageGroupProxyModel::groupByPage( bool value )
{
  if ( mGroupByPage == value )
    return;

  mGroupByPage = value;

  rebuildIndexes();
}


class AuthorGroupItem
{
    public:
        enum Type
        {
            Page,
            Author,
            Annotation
        };

        AuthorGroupItem( AuthorGroupItem *parent, Type type = Page, const QModelIndex &index = QModelIndex() )
            : mParent( parent ), mType( type ), mIndex( index )
        {
        }

        ~AuthorGroupItem()
        {
            qDeleteAll( mChilds );
        }

        void appendChild( AuthorGroupItem *child ) { mChilds.append( child ); }
        AuthorGroupItem* parent() const { return mParent; }
        AuthorGroupItem* child( int row ) const { return mChilds.value( row ); }
        int childCount() const { return mChilds.count(); }

        void dump( int level = 0 )
        {
            QString prefix;
            for ( int i = 0; i < level; ++i ) prefix += ' ';

            qDebug( "%s%s", qPrintable( prefix ), ( mType == Page ? "Page" : (mType == Author ? "Author" : "Annotation") ) );

            for ( int i = 0; i < mChilds.count(); ++i )
                mChilds[ i ]->dump( level + 2 );
        }

        const AuthorGroupItem* findIndex( const QModelIndex &index ) const
        {
            if ( index == mIndex )
                return this;

            for ( int i = 0; i < mChilds.count(); ++i ) {
                const AuthorGroupItem *item = mChilds[ i ]->findIndex( index );
                if ( item )
                    return item;
            }

            return 0;
        }

        int row() const
        {
            return ( mParent ? mParent->mChilds.indexOf( const_cast<AuthorGroupItem*>( this ) ) : 0 );
        }   

        Type type() const { return mType; }
        QModelIndex index() const { return mIndex; }

        void setAuthor( const QString &author ) { mAuthor = author; }
        QString author() const { return mAuthor; }

    private:
        AuthorGroupItem *mParent;
        Type mType;
        QModelIndex mIndex;
        QList<AuthorGroupItem*> mChilds;
        QString mAuthor;
};

class AuthorGroupProxyModel::Private
{
    public:
        Private( AuthorGroupProxyModel *parent )
            : mParent( parent ), mRoot( 0 ),
            mGroupByAuthor( false )
        {
        }
        ~Private()
        {
            delete mRoot;
        }

        AuthorGroupProxyModel *mParent;
        AuthorGroupItem *mRoot;
        bool mGroupByAuthor;
};

AuthorGroupProxyModel::AuthorGroupProxyModel( QObject *parent )
    : QAbstractProxyModel( parent ),
      d( new Private( this ) )
{
}

AuthorGroupProxyModel::~AuthorGroupProxyModel()
{
    delete d;
}

int AuthorGroupProxyModel::columnCount( const QModelIndex& ) const
{
    return 1;
}

int AuthorGroupProxyModel::rowCount( const QModelIndex &parentIndex ) const
{
    AuthorGroupItem *item = 0;
    if ( !parentIndex.isValid() )
        item = d->mRoot;
    else
        item = static_cast<AuthorGroupItem*>( parentIndex.internalPointer() );

    return item ? item->childCount() : 0;
}

QModelIndex AuthorGroupProxyModel::index( int row, int column, const QModelIndex &parentIndex ) const
{
    if ( !hasIndex( row, column, parentIndex ) )
        return QModelIndex();

    AuthorGroupItem *parentItem = 0;
    if ( !parentIndex.isValid() )
        parentItem = d->mRoot;
    else
        parentItem = static_cast<AuthorGroupItem*>( parentIndex.internalPointer() );

    AuthorGroupItem *child = parentItem->child( row );
    if ( child )
        return createIndex( row, column, child );
    else
        return QModelIndex();
}

QModelIndex AuthorGroupProxyModel::parent( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    AuthorGroupItem *childItem = static_cast<AuthorGroupItem*>( index.internalPointer() );
    AuthorGroupItem *parentItem = childItem->parent();

    if ( parentItem == d->mRoot )
        return QModelIndex();
    else
        return createIndex( parentItem->row(), 0, parentItem );
}

QModelIndex AuthorGroupProxyModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
    if ( !sourceIndex.isValid() )
        return QModelIndex();

    const AuthorGroupItem *item = d->mRoot->findIndex( sourceIndex );
    if ( !item )
        return QModelIndex();

    return createIndex( item->row(), 0, const_cast<AuthorGroupItem*>( item ) );
}

QModelIndex AuthorGroupProxyModel::mapToSource( const QModelIndex &proxyIndex ) const
{
    if ( !proxyIndex.isValid() )
        return QModelIndex();

    AuthorGroupItem *item = static_cast<AuthorGroupItem*>( proxyIndex.internalPointer() );

    return item->index();
}

void AuthorGroupProxyModel::setSourceModel( QAbstractItemModel *model )
{
    if ( sourceModel() ) {
        disconnect( sourceModel(), SIGNAL(layoutChanged()), this, SLOT(rebuildIndexes()) );
        disconnect( sourceModel(), SIGNAL(modelReset()), this, SLOT(rebuildIndexes()) );
        disconnect( sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rebuildIndexes()) );
        disconnect( sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rebuildIndexes()) );
    }

    QAbstractProxyModel::setSourceModel( model );

    connect( sourceModel(), SIGNAL(layoutChanged()), this, SLOT(rebuildIndexes()) );
    connect( sourceModel(), SIGNAL(modelReset()), this, SLOT(rebuildIndexes()) );
    connect( sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rebuildIndexes()) );
    connect( sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rebuildIndexes()) );

    rebuildIndexes();
}

static bool isAuthorItem( const QModelIndex &index )
{
    if ( !index.isValid() ) {
        return false;
    }

    AuthorGroupItem *item = static_cast<AuthorGroupItem*>( index.internalPointer() );
    return (item->type() == AuthorGroupItem::Author);
}

QItemSelection AuthorGroupProxyModel::mapSelectionToSource( const QItemSelection &selection ) const
{
    QModelIndexList proxyIndexes = selection.indexes();
    QItemSelection sourceSelection;
    for ( int i = 0; i < proxyIndexes.size(); ++i ) {
        if ( !isAuthorItem( proxyIndexes.at( i ) ) )
            sourceSelection << QItemSelectionRange( mapToSource( proxyIndexes.at( i ) ) );
    }

    return sourceSelection;
}

QItemSelection AuthorGroupProxyModel::mapSelectionFromSource( const QItemSelection &selection ) const
{
    return QAbstractProxyModel::mapSelectionFromSource( selection );
}

QVariant AuthorGroupProxyModel::data( const QModelIndex &proxyIndex, int role ) const
{
    if ( isAuthorItem( proxyIndex ) ) {
        AuthorGroupItem *item = static_cast<AuthorGroupItem*>( proxyIndex.internalPointer() );
        if ( role == Qt::DisplayRole )
            return item->author();
        else if ( role == Qt::DecorationRole )
            return KIcon( item->author().isEmpty() ? "user-away" : "user-identity" );
        else
            return QVariant();
    } else {
        return QAbstractProxyModel::data( proxyIndex, role );
    }
}

QMap<int, QVariant> AuthorGroupProxyModel::itemData( const QModelIndex &index ) const
{
    if ( isAuthorItem( index ) ) {
        return QMap<int, QVariant>();
    } else {
        return QAbstractProxyModel::itemData( index );
    }
}

Qt::ItemFlags AuthorGroupProxyModel::flags( const QModelIndex &index ) const
{
    if ( isAuthorItem( index ) ) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    } else {
        return QAbstractProxyModel::flags( index );
    }
}

void AuthorGroupProxyModel::groupByAuthor( bool value )
{
    if ( d->mGroupByAuthor == value )
        return;

    d->mGroupByAuthor = value;

    rebuildIndexes();
}

void AuthorGroupProxyModel::rebuildIndexes()
{
    delete d->mRoot;
    d->mRoot = new AuthorGroupItem( 0 );

    if ( d->mGroupByAuthor ) {
        QMap<QString, AuthorGroupItem*> authorMap;

        for ( int row = 0; row < sourceModel()->rowCount(); ++row ) {
            const QModelIndex idx = sourceModel()->index( row, 0 );
            const QString author = sourceModel()->data( idx, AnnotationModel::AuthorRole ).toString();
            if ( !author.isEmpty() ) {
                // We have the annotations as top-level, so introduce authors as new
                // top-levels and append the annotations
                AuthorGroupItem *authorItem = authorMap.value( author, 0 );
                if ( !authorItem ) {
                    authorItem = new AuthorGroupItem( d->mRoot, AuthorGroupItem::Author );
                    authorItem->setAuthor( author );

                    // Add item to tree
                    d->mRoot->appendChild( authorItem );

                    // Insert to lookup list
                    authorMap.insert( author, authorItem );
                }

                AuthorGroupItem *item = new AuthorGroupItem( authorItem, AuthorGroupItem::Annotation, idx );
                authorItem->appendChild( item );
            } else {
                // We have the pages as top-level, so we use them as top-level, append the
                // authors for all annotations of the page, and then the annotations themself
                AuthorGroupItem *pageItem = new AuthorGroupItem( d->mRoot, AuthorGroupItem::Page, idx );
                d->mRoot->appendChild( pageItem );

                // First collect all authors...
                QMap<QString, AuthorGroupItem*> pageAuthorMap;
                for ( int subRow = 0; subRow < sourceModel()->rowCount( idx ); ++subRow ) {
                    const QModelIndex annIdx = sourceModel()->index( subRow, 0, idx );
                    const QString author = sourceModel()->data( annIdx, AnnotationModel::AuthorRole ).toString();

                    AuthorGroupItem *authorItem = pageAuthorMap.value( author, 0 );
                    if ( !authorItem ) {
                        authorItem = new AuthorGroupItem( pageItem, AuthorGroupItem::Author );
                        authorItem->setAuthor( author );

                        // Add item to tree
                        pageItem->appendChild( authorItem );

                        // Insert to lookup list
                        pageAuthorMap.insert( author, authorItem );
                    }

                    AuthorGroupItem *item = new AuthorGroupItem( authorItem, AuthorGroupItem::Annotation, annIdx );
                    authorItem->appendChild( item );
                }
            }
        }
    } else {
        for ( int row = 0; row < sourceModel()->rowCount(); ++row ) {
            const QModelIndex idx = sourceModel()->index( row, 0 );
            const QString author = sourceModel()->data( idx, AnnotationModel::AuthorRole ).toString();
            if ( !author.isEmpty() ) {
                // We have the annotations as top-level items
                AuthorGroupItem *item = new AuthorGroupItem( d->mRoot, AuthorGroupItem::Annotation, idx );
                d->mRoot->appendChild( item );
            } else {
                // We have the pages as top-level items
                AuthorGroupItem *pageItem = new AuthorGroupItem( d->mRoot, AuthorGroupItem::Page, idx );
                d->mRoot->appendChild( pageItem );

                // Append all annotations as second-level
                for ( int subRow = 0; subRow < sourceModel()->rowCount( idx ); ++subRow ) {
                    const QModelIndex subIdx = sourceModel()->index( subRow, 0, idx );
                    AuthorGroupItem *item = new AuthorGroupItem( pageItem, AuthorGroupItem::Annotation, subIdx );
                    pageItem->appendChild( item );
                }
            }
        }
    }

    reset();
}

#include "annotationproxymodels.moc"
