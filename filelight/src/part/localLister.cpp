/***********************************************************************
* Copyright 2003-2004  Max Howell <max.howell@methylblue.com>
* Copyright 2008-2009  Martin Sandsmark <martin.sandsmark@kde.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "config-filelight.h"

#include "localLister.h"

#include "Config.h"
#include "fileTree.h"
#include "scan.h"

#include <kde_file.h>
#include <KDebug>
#include <Solid/StorageVolume>
#include <Solid/StorageAccess>
#include <Solid/Device>

#include <QtGui/QApplication> //postEvent()
#include <QtCore/QFile>
#include <QtCore/QByteArray>

#include <sys/stat.h> // S_BLKSIZE
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#ifndef S_BLKSIZE
#define S_BLKSIZE 512
#endif

namespace Filelight
{
QStringList LocalLister::s_remoteMounts;
QStringList LocalLister::s_localMounts;

LocalLister::LocalLister(const QString &path, Chain<Folder> *cachedTrees, ScanManager *parent)
        : QThread()
        , m_path(path)
        , m_trees(cachedTrees)
        , m_parent(parent)
{
    //add empty directories for any mount points that are in the path
    //TODO empty directories is not ideal as adds to fileCount incorrectly

    QStringList list(Config::skipList);
    if (!Config::scanAcrossMounts) list += s_localMounts;
    if (!Config::scanRemoteMounts) list += s_remoteMounts;

    foreach(const QString &ignorePath, list) {
        if (ignorePath.startsWith(path)) {
            m_trees->append(new Folder(ignorePath.toLocal8Bit()));
        }
    }
}

void
LocalLister::run()
{
    //recursively scan the requested path
    const QByteArray path = QFile::encodeName(m_path);
    Folder *tree = scan(path, path);

    //delete the list of trees useful for this scan,
    //in a sucessful scan the contents would now be transferred to 'tree'
    delete m_trees;

    if (m_parent->m_abort) //scan was cancelled
    {
        kDebug() << "Scan successfully aborted";
        delete tree;
        tree = 0;
    }
    kDebug() << "Emitting signal to cache results ...";
    emit branchCompleted(tree, true);
    kDebug() << "Thread terminating ...";
}

static void
outputError(QByteArray path)
{
    ///show error message that stat or opendir may give

#define LL_ERROR(s) kError() << s ": " << path; break

    switch (errno) {
    case EACCES:
        LL_ERROR("Inadequate access permissions");
    case EMFILE:
        LL_ERROR("Too many file descriptors in use by Filelight");
    case ENFILE:
        LL_ERROR("Too many files are currently open in the system");
    case ENOENT:
        LL_ERROR("A component of the path does not exist, or the path is an empty string");
    case ENOMEM:
        LL_ERROR("Insufficient memory to complete the operation");
    case ENOTDIR:
        LL_ERROR("A component of the path is not a folder");
    case EBADF:
        LL_ERROR("Bad file descriptor");
    case EFAULT:
        LL_ERROR("Bad address");
    case ELOOP: //NOTE shouldn't ever happen
        LL_ERROR("Too many symbolic links encountered while traversing the path");
    case ENAMETOOLONG:
        LL_ERROR("File name too long");
    }

#undef LL_ERROR
}

Folder*
LocalLister::scan(const QByteArray &path, const QByteArray &dirname)
{
    Folder *cwd = new Folder(dirname);
    DIR *dir = opendir(path);

    if (!dir) {
        outputError(path);
        return cwd;
    }

    KDE_struct_stat statbuf;
    KDE_struct_dirent *ent;
    while ((ent = KDE_readdir(dir)))
    {
        if (m_parent->m_abort)
            return cwd;

        if (qstrcmp(ent->d_name, ".") == 0 || qstrcmp(ent->d_name, "..") == 0)
            continue;

        QByteArray new_path = path;
        new_path += ent->d_name;

#ifdef HAVE_DIRENT_D_TYPE
        if (ent->d_type != DT_DIR && ent->d_type != DT_REG) {
            continue;
        }
#endif

        //get file information
        if (KDE_lstat(new_path, &statbuf) == -1) {
            outputError(new_path);
            continue;
        }

#ifndef HAVE_DIRENT_D_TYPE // anything that is not file/directory is filtered above
        if (S_ISLNK(statbuf.st_mode) ||
                S_ISCHR(statbuf.st_mode) ||
                S_ISBLK(statbuf.st_mode) ||
                S_ISFIFO(statbuf.st_mode)||
                S_ISSOCK(statbuf.st_mode))
        {
            continue;
        }
#endif

        if (S_ISREG(statbuf.st_mode)) //file
            cwd->append(ent->d_name, (statbuf.st_blocks * S_BLKSIZE));

        else if (S_ISDIR(statbuf.st_mode)) //folder
        {
            Folder *d = 0;
            QByteArray new_dirname = ent->d_name;
            new_dirname += '/';
            new_path += '/';

            //check to see if we've scanned this section already

            for (Iterator<Folder> it = m_trees->iterator(); it != m_trees->end(); ++it)
            {
                if (new_path == (*it)->name8Bit())
                {
                    kDebug() << "Tree pre-completed: " << (*it)->name();
                    d = it.remove();
                    m_parent->m_files += d->children();
                    //**** ideally don't have this redundant extra somehow
                    cwd->append(d, new_dirname);
                }
            }

            if (!d) //then scan
                if ((d = scan(new_path, new_dirname))) //then scan was successful
                    cwd->append(d);
        }

        ++m_parent->m_files;
    }

    closedir(dir);

    return cwd;
}

void LocalLister::readMounts()
{
    const Solid::StorageAccess *partition;
    const Solid::StorageVolume *volume;
    QStringList remoteFsTypes;
    remoteFsTypes << QLatin1String( "smbfs" ) << QLatin1String( "nfs" ) << QLatin1String( "afs" ); //TODO: expand

    foreach (const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess))
    { // Iterate over all the partitions available.
        if (!device.is<Solid::StorageAccess>()) continue; // It happens.
        if (!device.is<Solid::StorageVolume>()) continue;

        partition = device.as<Solid::StorageAccess>();
        if (!partition->isAccessible() || partition->filePath() == QLatin1String( "/" ) || partition->filePath().isEmpty()) continue;

        QString filePath = partition->filePath();
        if (!filePath.endsWith(QLatin1String("/")))
            filePath.append(QLatin1String("/"));
        volume = device.as<Solid::StorageVolume>();
        if (remoteFsTypes.contains(volume->fsType())) {
                if (!s_remoteMounts.contains(filePath)) {
                    s_remoteMounts.append(filePath);
                }
        } else if (!s_localMounts.contains(filePath)) {
            s_localMounts.append(filePath);
        }
    }

    kDebug() << "Found the following remote filesystems: " << s_remoteMounts;
    kDebug() << "Found the following local filesystems: " << s_localMounts;
}

}//namespace Filelight

#include "moc_localLister.cpp"

