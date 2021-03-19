/**
 Copyright (C) 2008  Unai Garro <ugarro@gmail.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 USA
**/

#include "rawcreator.h"

#include <QImage>

#include <libkdcraw/kdcraw.h>
#include <libkexiv2/kexiv2.h>
#include <kdemacros.h>

extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new RAWCreator;
    }
}

RAWCreator::RAWCreator()
{
}

RAWCreator::~RAWCreator()
{
}

bool RAWCreator::create(const QString &path, int width, int height, QImage &img)
{
    //load the image into the QByteArray
    QByteArray data;
    bool loaded=KDcrawIface::KDcraw::loadEmbeddedPreview(data,path);

    if (loaded)
    {

        //Load the image into a QImage
        QImage preview;
        if (!preview.loadFromData(data) || preview.isNull())
           return false;

        //And its EXIF info
        KExiv2Iface::KExiv2 exiv;
        if (exiv.loadFromData(data))
        {
            //We managed reading the EXIF info, rotate the image
            //according to the EXIF orientation flag
            exiv.rotateExifQImage(preview, exiv.getImageOrientation());
        }

        //Scale the image as requested by the thumbnailer
        img=preview.scaled(width,height,Qt::KeepAspectRatio);

    }
    return loaded;
}

ThumbCreator::Flags RAWCreator::flags() const
{
    return (Flags)(0);
}
