/***************************************************************************
 *   Copyright  2010 by Davide Bettio <davide.bettio@kdemail.net>          *
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

#include "imageloader.h"

#include <QImage>
#include <kdebug.h>

#ifdef HAVE_KEXIV2
#include <libkexiv2/kexiv2.h>
#endif

ImageLoader::ImageLoader(const QString &path)
{
    m_path = path;
#ifdef HAVE_KEXIV2 
    // prevets crashes due the xmp library not having a thread safe init
    // may get fixed in future version of exiv2 according to devs
    // FIXME: due to not knowing when it is safe to *uninitialize* exiv2 since others
    // may be using it, this may ultimately result in a small one-time memory leak;
    // either we need to know all users of KExiv2, or the thread safety issue needs
    // to get fixed.
    KExiv2Iface::KExiv2::initializeExiv2();
#endif
}

void ImageLoader::correctRotation(QImage& image, const QString &path)
{
#ifdef HAVE_KEXIV2
    if (!image.isNull()) {
        KExiv2Iface::KExiv2 exiv(path);
        exiv.rotateExifQImage(image, exiv.getImageOrientation());
#endif
    }
}

void ImageLoader::run()
{
    QImage img(m_path);
    correctRotation(img, m_path);
    emit loaded(img);
}

#include "moc_imageloader.cpp"
