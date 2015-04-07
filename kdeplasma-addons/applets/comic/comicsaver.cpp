/***************************************************************************
 *   Copyright (C) 2008-2012 Matthias Fuchs <mat69@gmx.net>                *
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

#include "comicsaver.h"
#include "comicdata.h"
#include "comicinfo.h"

#include <KFileDialog>
#include <KIO/NetAccess>
#include <KTemporaryFile>

ComicSaver::ComicSaver(SavingDir *savingDir)
  : mSavingDir(savingDir)
{
}

bool ComicSaver::save(const ComicData &comic)
{
    KTemporaryFile tempFile;

    if (!tempFile.open()) {
        return false;
    }

    // save image to temporary file
    comic.image().save(tempFile.fileName(), "PNG");

    KUrl srcUrl( tempFile.fileName() );

    const QString title = comic.title();

    const QString name = title + " - " + comic.current() + ".png";
    KUrl destUrl = KUrl(mSavingDir->getDir());
    destUrl.addPath( name );

    destUrl = KFileDialog::getSaveUrl( destUrl, "*.png" );
    if ( !destUrl.isValid() ) {
        return false;
    }

   mSavingDir->setDir(destUrl.directory());

    KIO::NetAccess::file_copy( srcUrl, destUrl );

    return true;
}
