
/***************************************************************************
 *   This file is part of the AudioThumbs.                                 *
 *   Copyright (C) 2009 Vytautas Mickus <vmickus@gmail.com>                *
 *   Copyright (C) 2011 Raizner Evgeniy <razrfalcon@gmail.com>             *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include "AudioThumbs.h"

#include <QImage>
#include <QFile>

#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/mp4file.h>
#include <taglib/mp4properties.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>
#include <taglib/xiphcomment.h>
#include <taglib/xingheader.h>
#include <taglib/oggflacfile.h>
#include <taglib/oggfile.h>
#include <taglib/vorbisfile.h>

#include <kmimetype.h>
#include <kdemacros.h>

extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new ATCreator();
    }
}


ATCreator::ATCreator()
{
}

bool ATCreator::create(const QString &path, int /*w*/, int /*h*/, QImage &img)
{
    bool bRet = false;

    KMimeType::Ptr type = KMimeType::findByPath(path, 0, true);

    if (type->is("audio/mpeg")) {
        TagLib::MPEG::File mp3File(path.toUtf8());
        TagLib::ID3v2::Tag *mp3Tag = mp3File.ID3v2Tag();
        TagLib::ID3v2::FrameList fList = mp3Tag->frameList("APIC");
        TagLib::ID3v2::AttachedPictureFrame *pic;
        pic = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(fList.front());
        if (pic && !pic->picture().isEmpty()) {
            img.loadFromData(pic->picture().data(),pic->picture().size());
            bRet = true;
        }

    } else if (type->is("audio/flac") || type->is("audio/x-flac")) {
        TagLib::FLAC::File ff(path.toUtf8());
        TagLib::List<TagLib::FLAC::Picture *> coverList;

        if (!ff.pictureList().isEmpty()) {
            coverList.append(ff.pictureList().front());
            TagLib::ByteVector coverData = coverList.front()->data();
            img.loadFromData(coverData.data(),coverData.size());
            bRet = true;
            return bRet;
        }

    } else if (type->is("audio/mp4")) {
        TagLib::MP4::File mp4file(path.toUtf8());
        TagLib::MP4::Tag *tag = mp4file.tag();
        TagLib::MP4::ItemListMap map=tag->itemListMap();

        if (!map.isEmpty()) {
            for (TagLib::MP4::ItemListMap::ConstIterator it = map.begin(); it != map.end(); ++it) {
                TagLib::MP4::CoverArtList coverList=(*it).second.toCoverArtList();
                    if (!coverList.isEmpty()) {
                        TagLib::MP4::CoverArt cover=coverList[0];

                        TagLib::ByteVector coverData=cover.data();
                        img.loadFromData(coverData.data(),coverData.size());

                        bRet = true;
                        return bRet;
                  }
              }
         }
    } else if (type->is("audio/x-vorbis+ogg") || type->is("audio/ogg")) {
        TagLib::Ogg::Vorbis::File file(path.toUtf8());
        TagLib::Ogg::XiphComment *tag = file.tag();

        TagLib::List<TagLib::FLAC::Picture*> picturelist = tag->pictureList();
        for (int i = 0; i < picturelist.size(); i++) {
            // qDebug() << Q_FUNC_INFO << picturelist[i]->code();
            switch (picturelist[i]->code()) {
                case TagLib::FLAC::Picture::FrontCover:
                case TagLib::FLAC::Picture::BackCover:
                case TagLib::FLAC::Picture::Media: {
                    break;
                }
                default: {
                    continue;
                    break;
                }
            }

            img.loadFromData(picturelist[i]->data().data(), picturelist[i]->data().size());
            if (!img.isNull()) {
                bRet = true;
                break;
            }
        }
    }
    
    return bRet;
}

ThumbCreator::Flags ATCreator::flags() const
{
    return (Flags)(None);
}

