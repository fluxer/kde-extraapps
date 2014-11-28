/****************************************************************************************
 * Copyright (c) 2010 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
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

#ifndef COLLECTIONTESTIMPL_H
#define COLLECTIONTESTIMPL_H

#include "core/collections/Collection.h"
#include "core-impl/collections/support/MemoryCollection.h"
#include "core-impl/collections/support/MemoryQueryMaker.h"

#include <KIcon>

#include <QSharedPointer>

//simple Collections::Collection implementation based on MemoryCollection

class CollectionLocationTestImpl;

namespace Collections {

class CollectionTestImpl : public Collections::Collection
{
public:
    CollectionTestImpl( const QString &collectionId )
        : Collections::Collection(), mc( new MemoryCollection() )
    { this->id = collectionId; }

    QueryMaker* queryMaker() { return new MemoryQueryMaker( mc.toWeakRef(), id ); }

    KIcon icon() const { return KIcon(); }

    QString collectionId() const { return id; }
    QString prettyName() const { return id; }

    QString id;

    QSharedPointer<MemoryCollection> mc;

};

} //namespace Collections

#endif
