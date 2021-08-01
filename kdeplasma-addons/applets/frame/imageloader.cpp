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
#include <kexiv2.h>

ImageLoader::ImageLoader(const QString &path)
{
    m_path = path;
}

void ImageLoader::correctRotation(QImage& image, const QString &path)
{
    if (!image.isNull()) {
        KExiv2 exiv(path);
        exiv.rotateImage(image);
    }
}

void ImageLoader::run()
{
    QImage img(m_path);
    correctRotation(img, m_path);
    emit loaded(img);
}

#include "moc_imageloader.cpp"
