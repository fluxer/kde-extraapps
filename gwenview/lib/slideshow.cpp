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
#include "moc_slideshow.cpp"

// Qt
#include <QAction>
#include <QTimer>

// KDE
#include <KConfig>
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KRandom>

// Local
#include <lib/gvdebug.h>
#include <gwenviewconfig.h>

namespace Gwenview
{

enum State {
    Stopped,
    Started,
    WaitForEndOfUrl
};

struct SlideShowPrivate
{
    QTimer* mTimer;
    State mState;
    QList<KUrl> mUrls;
    QList<KUrl> mShuffledUrls;
    QList<KUrl>::ConstIterator mStartIt;
    KUrl mCurrentUrl;
    KUrl mLastShuffledUrl;

    QAction* mLoopAction;
    QAction* mRandomAction;

    KUrl findNextUrl()
    {
        if (GwenviewConfig::random()) {
            return findNextRandomUrl();
        } else {
            return findNextOrderedUrl();
        }
    }

    KUrl findNextOrderedUrl()
    {
        QList<KUrl>::ConstIterator it = qFind(mUrls.constBegin(), mUrls.constEnd(), mCurrentUrl);
        GV_RETURN_VALUE_IF_FAIL2(it != mUrls.constEnd(), KUrl(), "Current url not found in list.");

        ++it;
        if (GwenviewConfig::loop()) {
            // Looping, if we reach the end, start again
            if (it == mUrls.constEnd()) {
                it = mUrls.constBegin();
            }
        } else {
            // Not looping, have we reached the end?
            // FIXME: stopAtEnd
            if (/*(it==mUrls.end() && GwenviewConfig::stopAtEnd()) ||*/ it == mStartIt) {
                it = mUrls.constEnd();
            }
        }

        if (it != mUrls.constEnd()) {
            return *it;
        } else {
            return KUrl();
        }
    }

    void initShuffledUrls()
    {
        mShuffledUrls = mUrls;
        KRandom::randomize(mShuffledUrls);
        // Ensure the first url is different from the previous last one, so that
        // last url does not stay visible twice longer than usual
        if (mLastShuffledUrl == mShuffledUrls.first() && mShuffledUrls.count() > 1) {
            qSwap(mShuffledUrls[0], mShuffledUrls[1]);
        }
        mLastShuffledUrl = mShuffledUrls.last();
    }

    KUrl findNextRandomUrl()
    {
        if (mShuffledUrls.empty()) {
            if (GwenviewConfig::loop()) {
                initShuffledUrls();
            } else {
                return KUrl();
            }
        }

        KUrl url = mShuffledUrls.last();
        mShuffledUrls.pop_back();

        return url;
    }

    void updateTimerInterval()
    {
        mTimer->setInterval(int(GwenviewConfig::interval() * 1000));
    }

    void doStart()
    {
        kDebug() << "mState = Started";
        mTimer->start();
        mState = Started;
    }
};

SlideShow::SlideShow(QObject* parent)
: QObject(parent)
, d(new SlideShowPrivate)
{
    d->mState = Stopped;

    d->mTimer = new QTimer(this);
    connect(d->mTimer, SIGNAL(timeout()),
            this, SLOT(goToNextUrl()));

    d->mLoopAction = new QAction(this);
    d->mLoopAction->setText(i18nc("@item:inmenu toggle loop in slideshow", "Loop"));
    d->mLoopAction->setCheckable(true);
    connect(d->mLoopAction, SIGNAL(triggered()), SLOT(updateConfig()));

    d->mRandomAction = new QAction(this);
    d->mRandomAction->setText(i18nc("@item:inmenu toggle random order in slideshow", "Random"));
    d->mRandomAction->setCheckable(true);
    connect(d->mRandomAction, SIGNAL(toggled(bool)), SLOT(slotRandomActionToggled(bool)));
    connect(d->mRandomAction, SIGNAL(triggered()), SLOT(updateConfig()));

    d->mLoopAction->setChecked(GwenviewConfig::loop());
    d->mRandomAction->setChecked(GwenviewConfig::random());
}

SlideShow::~SlideShow()
{
    GwenviewConfig::self()->writeConfig();
    delete d;
}

QAction* SlideShow::loopAction() const
{
    return d->mLoopAction;
}

QAction* SlideShow::randomAction() const
{
    return d->mRandomAction;
}

void SlideShow::start(const QList<KUrl>& urls)
{
    d->mUrls = urls;

    d->mStartIt = qFind(d->mUrls.constBegin(), d->mUrls.constEnd(), d->mCurrentUrl);
    if (d->mStartIt == d->mUrls.constEnd()) {
        kWarning() << "Current url not found in list, aborting.\n";
        return;
    }

    if (GwenviewConfig::random()) {
        d->initShuffledUrls();
    }

    d->updateTimerInterval();
    d->mTimer->setSingleShot(false);
    d->doStart();
    stateChanged(true);
}

void SlideShow::setInterval(int intervalInSeconds)
{
    GwenviewConfig::setInterval(double(intervalInSeconds));
    d->updateTimerInterval();
}

void SlideShow::stop()
{
    kDebug() << "Stopping timer";
    d->mTimer->stop();
    d->mState = Stopped;
    stateChanged(false);
}

void SlideShow::resumeAndGoToNextUrl()
{
    if (d->mState == WaitForEndOfUrl) {
        goToNextUrl();
    }
}

void SlideShow::goToNextUrl()
{
    KUrl url = d->findNextUrl();
    kDebug() << "Next url:" << url;
    if (!url.isValid()) {
        stop();
        return;
    }
    goToUrl(url);
}

void SlideShow::setCurrentUrl(const KUrl& url)
{
    kDebug() << "Current url" << url;
    if (d->mCurrentUrl == url) {
        return;
    }
    d->mCurrentUrl = url;
    // Restart timer to avoid showing new url for the remaining time of the old
    // url
    if (d->mState != Stopped) {
        d->doStart();
    }
}

bool SlideShow::isRunning() const
{
    return d->mState != Stopped;
}

void SlideShow::updateConfig()
{
    GwenviewConfig::setLoop(d->mLoopAction->isChecked());
    GwenviewConfig::setRandom(d->mRandomAction->isChecked());
}

void SlideShow::slotRandomActionToggled(bool on)
{
    if (on && d->mState != Stopped) {
        d->initShuffledUrls();
    }
}

} // namespace
