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

#include "config-ffmpegthumbnailer.h"
#include "ffmpegthumbnailer.h"
#include "ffmpegthumbssettings.h"
#include "ui_ffmpegthumbsform.h"

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
            break;
        }
        case ThumbnailerLogLevel::ThumbnailerLogLevelError: {
            kError() << ffmpegmessage;
            break;
        }
    }
}

class FFMpegThumbsFormWidget : public QWidget, public Ui::FFMpegThumbsForm
{
    Q_OBJECT
public:
    FFMpegThumbsFormWidget() { setupUi(this); }
};


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

    FFMpegThumbsSettings* ffmpegthumbssettings = FFMpegThumbsSettings::self();
    ffmpegthumbssettings->readConfig();

    const QByteArray pathbytes = QFile::encodeName(path);
#if defined(HAVE_VIDEO_THUMBNAILER_SET_SIZE)
    video_thumbnailer_set_size(ffmpegthumb, width, heigth);
#else
    ffmpegthumb->thumbnail_size = qMax(width, heigth);
#endif
    ffmpegthumb->seek_percentage = 20;
    ffmpegthumb->overlay_film_strip = ffmpegthumbssettings->film_strip();
#if defined(HAVE_PREFER_EMBEDDED_METADATA)
    ffmpegthumb->prefer_embedded_metadata = ffmpegthumbssettings->prefer_embedded_metadata();
#endif
    ffmpegthumb->workaround_bugs = ffmpegthumbssettings->workaround_bugs();
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
    return ThumbCreator::None;
}

QWidget *FFMpegThumbnailer::createConfigurationWidget()
{
    FFMpegThumbsSettings* ffmpegthumbssettings = FFMpegThumbsSettings::self();
    FFMpegThumbsFormWidget* ffmpegthumbsformwidget = new FFMpegThumbsFormWidget();
    ffmpegthumbsformwidget->filmStripCheckBox->setChecked(ffmpegthumbssettings->film_strip());
    ffmpegthumbsformwidget->preferEmbeddedMetadataCheckBox->setChecked(ffmpegthumbssettings->prefer_embedded_metadata());
    ffmpegthumbsformwidget->workaroundBugsCheckBox->setChecked(ffmpegthumbssettings->workaround_bugs());
#if !defined(HAVE_PREFER_EMBEDDED_METADATA)
    ffmpegthumbsformwidget->preferEmbeddedMetadataCheckBox->setVisible(false);
#endif
    return ffmpegthumbsformwidget;
}

void FFMpegThumbnailer::writeConfiguration(const QWidget *configurationWidget)
{
    const FFMpegThumbsFormWidget *ffmpegthumbsformwidget = qobject_cast<const FFMpegThumbsFormWidget*>(configurationWidget);
    Q_ASSERT(ffmpegthumbsformwidget);
    FFMpegThumbsSettings* ffmpegthumbssettings = FFMpegThumbsSettings::self();
    ffmpegthumbssettings->setFilm_strip(ffmpegthumbsformwidget->filmStripCheckBox->isChecked());
    ffmpegthumbssettings->setPrefer_embedded_metadata(ffmpegthumbsformwidget->preferEmbeddedMetadataCheckBox->isChecked());
    ffmpegthumbssettings->setWorkaround_bugs(ffmpegthumbsformwidget->workaroundBugsCheckBox->isChecked());
    ffmpegthumbssettings->writeConfig();
}

#include "ffmpegthumbnailer.moc"
