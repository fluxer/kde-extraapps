/*  This file is part of the KDE project
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ffmpegthumbnailer.h"

#include <QImage>
#include <kdebug.h>
#include <kdemacros.h>

extern "C"
{
    KDE_EXPORT ThumbCreator* new_creator()
    {
        return new FFMpegThumbnailer();
    }
}

FFMpegThumbnailer::FFMpegThumbnailer()
{
    m_Thumbnailer.addFilter(&m_FilmStrip);
}

bool FFMpegThumbnailer::create(const QString& path, int width, int /*heigth*/, QImage& img)
{
    std::vector<uint8_t> pixelBuffer;

    m_Thumbnailer.setThumbnailSize(width);
    // 20% seek inside the video to generate the preview
    m_Thumbnailer.setSeekPercentage(20);

    try {
        //Smart frame selection is very slow compared to the fixed detection
        //TODO: Use smart detection if the image is single colored.
        //m_Thumbnailer.setSmartFrameSelection(true);
        m_Thumbnailer.generateThumbnail(std::string(path.toUtf8()), Png, pixelBuffer);
    } catch(std::exception &err) {
        kWarning() << err.what();
        return false;
    } catch (...) {
        return false;
    }

    return img.loadFromData(reinterpret_cast<char*>(&pixelBuffer.front()), pixelBuffer.size(), "PNG");
}

ThumbCreator::Flags FFMpegThumbnailer::flags() const
{
    return ThumbCreator::DrawFrame;
}

