/**
 * Copyright (C) 2002-2004 Scott Wheeler <wheeler@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MEDIAFILES_H
#define MEDIAFILES_H

class QWidget;
class QString;
class QStringList;

namespace TagLib {
    class File;
}

#include <kurl.h>
#include <taglib_config.h>

/**
 * A namespace for file JuK's file related functions.  The goal is to hide
 * all specific knowledge of mimetypes and file extensions here.
 */
namespace MediaFiles
{
    /**
     * Creates a JuK specific KFileDialog with the specified parent.
     */
    QStringList openDialog(QWidget *parent = 0);

    /**
     * Creates a JuK specific KFileDialog for saving a playlist with the name
     * playlistName and the specified parent and returns the file name.
     */
    QString savePlaylistDialog(const QString &playlistName, QWidget *parent = 0);

    /**
     * Returns a pointer to a new appropriate subclass of TagLib::File, or
     * a null pointer if there is no appropriate subclass for the given
     * file.
     */
    TagLib::File *fileFactoryByType(const QString &fileName);

    /**
     * Returns true if fileName is a supported media file.
     */
    bool isMediaFile(const QString &fileName);

    /**
     * Returns true if fileName is a supported playlist file.
     */
    bool isPlaylistFile(const QString &fileName);

    /**
     * Returns a list of all supported mimetypes.
     */
    QStringList mimeTypes();

    /**
     * Returns true if fileName is a mp3 file.
     */
    bool isMP3(const QString &fileName);

    /**
     * Returns true if fileName is a mpc (aka musepack) file.
     */
    bool isMPC(const QString &fileName);

    /**
     * Returns true if fileName is an Ogg file.
     */
    bool isOgg(const QString &fileName);

    /**
     * Returns true if fileName is a FLAC file.
     */
    bool isFLAC(const QString &fileName);

    /**
     * Returns true if fileName is an Ogg/Vorbis file.
     */
    bool isVorbis(const QString &fileName);

#ifdef TAGLIB_WITH_ASF
    /**
     * Returns true if fileName is an ASF file.
     */
    bool isASF(const QString &fileName);
#endif

#ifdef TAGLIB_WITH_MP4
    /**
     * Returns true if fileName is a MP4 file.
     */
    bool isMP4(const QString &fileName);
#endif

    /**
     * Returns true if fileName is an Ogg/FLAC file.
     */
    bool isOggFLAC(const QString &fileName);

    /**
     * Returns a list of absolute local filenames, mapped from \p urlList.
     * Any URLs in urlList that aren't really local files will be stripped
     * from the result (so result.size() may be < urlList.size()).
     *
     * @param urlList list of file names or URLs to convert.
     * @param w KIO may need the widget to handle user interaction.
     * @return list of all local files in urlList, converted to absolute paths.
     */
    QStringList convertURLsToLocal(const KUrl::List &urlList, QWidget *w = 0);
}

#endif

// vim: set et sw=4 tw=0 sta:
