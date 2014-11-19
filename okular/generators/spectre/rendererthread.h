/***************************************************************************
 *   Copyright (C) 2007 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_GSRENDERERTHREAD_H_
#define _OKULAR_GSRENDERERTHREAD_H_

#include <qmutex.h>
#include <qqueue.h>
#include <qsemaphore.h>
#include <qstring.h>
#include <qthread.h>

#include <libspectre/spectre.h>

class QImage;
class GSGenerator;

namespace Okular
{
   class PixmapRequest;
}

struct GSRendererThreadRequest
{
    GSRendererThreadRequest(GSGenerator *_owner)
        : owner(_owner)
        , request(0)
        , spectrePage(0)
        , textAAbits(1)
        , graphicsAAbits(1)
        , magnify(1.0)
        , orientation(0)
        , platformFonts(true)
    {}

    GSGenerator *owner;
    Okular::PixmapRequest *request;
    SpectrePage *spectrePage;
    int textAAbits;
    int graphicsAAbits;
    double magnify;
    int orientation;
    bool platformFonts;
};
Q_DECLARE_TYPEINFO(GSRendererThreadRequest, Q_MOVABLE_TYPE);

class GSRendererThread : public QThread
{
Q_OBJECT
    public:
        static GSRendererThread *getCreateRenderer();

        ~GSRendererThread();

        void addRequest(const GSRendererThreadRequest &req);

    signals:
        void imageDone(QImage *image, Okular::PixmapRequest *request);

    private:
        GSRendererThread();

        QSemaphore m_semaphore;

        static GSRendererThread *theRenderer;

        void run();

        SpectreRenderContext *m_renderContext;
        QQueue<GSRendererThreadRequest> m_queue;
        QMutex m_queueMutex;
};

#endif
