/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#ifndef DOCUMENTTEST_H
#define DOCUMENTTEST_H

// Qt
#include <QObject>

// KDE
#include <KJob>

// Local
#include "../lib/document/document.h"

class LoadingStateSpy : public QObject
{
    Q_OBJECT
public:
    LoadingStateSpy(const Gwenview::Document::Ptr& doc)
        : mDocument(doc)
        , mCallCount(0) {
    }

public Q_SLOTS:
    void readState()
    {
        mCallCount++;
        mState = mDocument->loadingState();
    }

public:
    Gwenview::Document::Ptr mDocument;
    int mCallCount;
    Gwenview::Document::LoadingState mState;
};

class JobWatcher : public QObject
{
    Q_OBJECT
public:
    JobWatcher(KJob* job)
        : mJob(job)
        , mError(0) {
        job->setAutoDelete(false);
        connect(job, SIGNAL(result(KJob*)),
                SLOT(slotResult(KJob*)));
    }

    void wait()
    {
        mJob->exec();
    }

    int error() const
    {
        return mError;
    }

private Q_SLOTS:
    void slotResult(KJob* job)
    {
        mError = job->error();
        job->deleteLater();
    }

private:
    KJob* mJob;
    int mError;
};

class DocumentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLoad();
    void testLoad_data();
    void testLoadTwoPasses();
    void testLoadEmpty();
    void testLoadDownSampled();
    void testLoadDownSampled_data();
    void testLoadDownSampledPng();
    void testLoadRemote();
    void testLoadAnimated();
    void testPrepareDownSampledAfterFailure();
    void testDeleteWhileLoading();
    void testLoadRotated();
    void testMultipleLoads();
    void testSaveAs();
    void testSaveRemote();
    void testLosslessSave();
    void testLosslessRotate();
    void testModifyAndSaveAs();
    void testMetaInfoJpeg();
    void testForgetModifiedDocument();
    void testModifiedAndSavedSignals();
    void testJobQueue();
    void testCheckDocumentEditor();
    void testUndoStackPush();

    void initTestCase();
    void init();
};

#endif // DOCUMENTTEST_H
