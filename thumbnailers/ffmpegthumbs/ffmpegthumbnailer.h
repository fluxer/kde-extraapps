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

#ifndef FFMPEGTHUMBNAILER_H
#define FFMPEGTHUMBNAILER_H

#include <kio/thumbcreator.h>

class FFMpegThumbnailer : public ThumbCreator
{
public:
    FFMpegThumbnailer();

    bool create(const QString &path, int width, int height, QImage &img) final;
    ThumbCreator::Flags flags() const final;

    QWidget *createConfigurationWidget() final;
    void writeConfiguration(const QWidget *configurationWidget) final;
};

#endif // FFMPEGTHUMBNAILER_H
