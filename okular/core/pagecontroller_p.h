/***************************************************************************
 *   Copyright (C) 2007 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_PAGECONTROLLER_P_H_
#define _OKULAR_PAGECONTROLLER_P_H_

#include <QtCore/QObject>

namespace ThreadWeaver {
    class Job;
}

namespace Okular {

class Page;
class RotationJob;

/* There is one PageController per document. It receives notifications of
 * completed RotationJobs */
class PageController : public QObject
{
    Q_OBJECT

    public:
        PageController();
        ~PageController();

        void addRotationJob( RotationJob *job );

    signals:
        void rotationFinished( int page, Okular::Page *okularPage );

    private slots:
        void imageRotationDone(ThreadWeaver::Job*);
};

}

#endif
