/**
 * Copyright (C) 2004 Scott Wheeler <wheeler@kde.org>
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

#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include <QString>

class QFileInfo;
class QDateTime;
class QDataStream;
class QStringList;

class CoverInfo;
class Tag;
class CacheDataStream;

template<class T>
class QList;

/**
 * An value based, explicitly shared wrapper around file related information
 * used in JuK's playlists.
 */

class FileHandle
{
public:
    FileHandle();
    FileHandle(const FileHandle &f);
    explicit FileHandle(const QFileInfo &info, const QString &path = QString());
    explicit FileHandle(const QString &path);
    FileHandle(const QString &path, CacheDataStream &s);
    ~FileHandle();

    /**
     * Forces the FileHandle to reread its information from the disk.
     */
    void refresh();
    void setFile(const QString &path);

    Tag *tag() const;
    CoverInfo *coverInfo() const;
    QString absFilePath() const;
    const QFileInfo &fileInfo() const;

    bool isNull() const;
    bool current() const;
    const QDateTime &lastModified() const;

    void read(CacheDataStream &s);

    FileHandle &operator=(const FileHandle &f);
    bool operator==(const FileHandle &f) const;
    bool operator!=(const FileHandle &f) const;

    static QStringList properties();
    QString property(const QString &name) const;

    static const FileHandle &null();

private:
    class FileHandlePrivate;
    FileHandlePrivate *d;

    void setup(const QFileInfo &info, const QString &path);
};

typedef QList<FileHandle> FileHandleList;

QDataStream &operator<<(QDataStream &s, const FileHandle &f);
CacheDataStream &operator>>(CacheDataStream &s, FileHandle &f);

#endif

// vim: set et sw=4 tw=0 sta:
