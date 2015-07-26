/* This file is part of KDevelop
  Copyright 2005 Adam Treat <treat@kde.org>
  Copyright 2013 Sebastian Kügler <sebas@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kdevdocumentmodel.h"
#include <KFileItem>
#include <QtCore/qdebug.h>

KDevDocumentItem::KDevDocumentItem( const QString &name )
    : QStandardItem( name ),
        m_documentState( KDevelop::IDocument::Clean )
{
    setIcon( icon() );
}

KDevDocumentItem::~KDevDocumentItem()
{}

KDevCategoryItem::KDevCategoryItem( const QString &name )
        : KDevDocumentItem( name )
{
    setFlags(Qt::ItemIsEnabled);
    setToolTip( name );
    setIcon( KIcon("folder") );
}

KDevCategoryItem::~KDevCategoryItem()
{}

QList<KDevFileItem*> KDevCategoryItem::fileList() const
{
    QList<KDevFileItem*> lst;

    for ( int i = 0; i < rowCount(); ++i )
    {
        if ( KDevFileItem * item = dynamic_cast<KDevDocumentItem*>( child( i ) ) ->fileItem() )
            lst.append( item );
    }

    return lst;
}

KDevFileItem* KDevCategoryItem::file( const KUrl &url ) const
{
    foreach( KDevFileItem * item, fileList() )
    {
        if ( item->url() == url )
            return item;
    }

    return 0;
}

KDevFileItem::KDevFileItem( const KUrl &url )
        : KDevDocumentItem( url.fileName() )
{
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    setUrl( url );
    if (!url.isEmpty()) {
        m_fileIcon = KFileItem(url, QString(), 0).iconName();
    }
    setIcon( KIcon( m_fileIcon ) );
}

KDevFileItem::~KDevFileItem()
{}

KDevDocumentModel::KDevDocumentModel( QObject *parent )
    : QStandardItemModel( parent )
{
    setRowCount(0);
    setColumnCount(1);
}

KDevDocumentModel::~KDevDocumentModel()
{}

QList<KDevCategoryItem*> KDevDocumentModel::categoryList() const
{

    QList<KDevCategoryItem*> lst;
    for ( int i = 0; i < rowCount() ; ++i )
    {
        if ( KDevCategoryItem * categoryitem = dynamic_cast<KDevDocumentItem*>( item( i ) ) ->categoryItem() )
        {

            lst.append( categoryitem );
        }
    }

    return lst;
}

KDevCategoryItem* KDevDocumentModel::category( const QString& category ) const
{
    foreach( KDevCategoryItem * item, categoryList() )
    {
        if ( item->toolTip() == category )
            return item;
    }

    return 0;
}

#include "moc_kdevdocumentmodel.cpp"

