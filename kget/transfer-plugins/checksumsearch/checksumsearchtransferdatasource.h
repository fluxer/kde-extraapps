/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#ifndef CHECKSUMSEARCHTRANSFERDATASOURCE_H
#define CHECKSUMSEARCHTRANSFERDATASOURCE_H

#include "core/transferdatasource.h"

#include <KIO/Job>

class ChecksumSearch;
class ChecksumSearchTransferDataSource;

class ChecksumSearchTransferDataSource : public TransferDataSource
{
    Q_OBJECT
    public:
        ChecksumSearchTransferDataSource(const KUrl &srcUrl, QObject *parent);
        virtual ~ChecksumSearchTransferDataSource();

        void start();
        void stop();
        void addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange);

    private:
        KUrl m_src;

    friend class ChecksumSearchController;
};

#endif
