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

#include <QFile>
#include <QImage>
#include <kdebug.h>
#include <kdemacros.h>

#include <libffmpegthumbnailer/videothumbnailerc.h>

void ffmpeg_log_callback(ThumbnailerLogLevel ffmpegloglevel, const char* ffmpegmessage)
{
    switch (ffmpegloglevel) {
        case ThumbnailerLogLevel::ThumbnailerLogLevelInfo: {
            kDebug() << ffmpegmessage;
        }
        case ThumbnailerLogLevel::ThumbnailerLogLevelError: {
            kError() << ffmpegmessage;
        }
    }
}

extern "C"
{
    KDE_EXPORT ThumbCreator* new_creator()
    {
        return new FFMpegThumbnailer();
    }
}

FFMpegThumbnailer::FFMpegThumbnailer()
{
}

bool FFMpegThumbnailer::create(const QString &path, int width, int heigth, QImage &img)
{
    video_thumbnailer* ffmpegthumb = video_thumbnailer_create();
    if (!ffmpegthumb) {
        kWarning() << "Could not create video thumbnailer";
        return false;
    }

    video_thumbnailer_set_log_callback(ffmpegthumb, ffmpeg_log_callback);

    image_data* ffmpegimage = video_thumbnailer_create_image_data();
    if (!ffmpegimage) {
        kWarning() << "Could not create video thumbnailer image";
        video_thumbnailer_destroy(ffmpegthumb);
        return false;
    }

    const QByteArray pathbytes = QFile::encodeName(path);
    ffmpegthumb->thumbnail_size = qMax(width, heigth);
    ffmpegthumb->seek_percentage = 20;
    ffmpegthumb->overlay_film_strip = 1;
    ffmpegthumb->thumbnail_image_type = ThumbnailerImageType::Png;

    const int ffmpegresult = video_thumbnailer_generate_thumbnail_to_buffer(
        ffmpegthumb,
        pathbytes.constData(),
        ffmpegimage
    );
    if (ffmpegresult != 0) {
        kWarning() << "Could not generate video thumbnail";
        video_thumbnailer_destroy_image_data(ffmpegimage);
        video_thumbnailer_destroy(ffmpegthumb);
        return false;
    }

    img.loadFromData(
        reinterpret_cast<char*>(ffmpegimage->image_data_ptr),
        ffmpegimage->image_data_size,
        "PNG"
    );

    if (img.isNull()) {
        kWarning() << "Could not load the video thumbnail";
        video_thumbnailer_destroy_image_data(ffmpegimage);
        video_thumbnailer_destroy(ffmpegthumb);
        return false;
    }

    video_thumbnailer_destroy_image_data(ffmpegimage);
    video_thumbnailer_destroy(ffmpegthumb);
    return true;
}

ThumbCreator::Flags FFMpegThumbnailer::flags() const
{
    return ThumbCreator::DrawFrame;
}

