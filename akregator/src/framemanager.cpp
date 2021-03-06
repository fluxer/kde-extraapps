/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "framemanager.h"
#include "akregatorconfig.h"
#include "frame.h"
#include "actionmanager.h"
#include "openurlrequest.h"

#include <kaction.h>
#include <KCharMacroExpander>
#include <kprocess.h>
#include <kshell.h>
#include <kconfiggroup.h>
#include <ktoolinvocation.h>
#include <kparts/event.h>
#include <krun.h>

#include <QtCore/QStringList>
#include <QtGui/QApplication>

namespace Akregator {

static void setPartGuiActive(KParts::ReadOnlyPart* part, bool active)
{
    if (!part)
        return;
    // When this event is sent to a KPart, the KPart StatusBarExtension shows or
    // hides its items
    KParts::GUIActivateEvent ev(active);
    QApplication::sendEvent(part, &ev);
}

FrameManager::FrameManager(QWidget* mainWin, QObject* parent) : QObject(parent), m_mainWin(mainWin), m_currentFrame(0)
{
}

FrameManager::~FrameManager()
{
}

void FrameManager::setMainWindow(QWidget* mainWin)
{
    m_mainWin = mainWin;
}

Frame* FrameManager::currentFrame() const
{
    return m_currentFrame;
}

void FrameManager::slotAddFrame(Frame* frame)
{
    m_frames.insert(frame->id(), frame);

    connect(frame, SIGNAL(signalCanceled(Akregator::Frame*,QString)), this, SLOT(slotSetCanceled(Akregator::Frame*,QString)) );
    connect(frame, SIGNAL(signalStarted(Akregator::Frame*)), this, SLOT(slotSetStarted(Akregator::Frame*)) );
    connect(frame, SIGNAL(signalCaptionChanged(Akregator::Frame*,QString)), this, SLOT(slotSetCaption(Akregator::Frame*,QString)));
    connect(frame, SIGNAL(signalLoadingProgress(Akregator::Frame*,int)), this, SLOT(slotSetProgress(Akregator::Frame*,int)));
    connect(frame, SIGNAL(signalCompleted(Akregator::Frame*)), this, SLOT(slotSetCompleted(Akregator::Frame*)));
    connect(frame, SIGNAL(signalTitleChanged(Akregator::Frame*,QString)), this, SLOT(slotSetTitle(Akregator::Frame*,QString)) );
    connect(frame, SIGNAL(signalStatusText(Akregator::Frame*,QString)), this, SLOT(slotSetStatusText(Akregator::Frame*,QString))
);

    connect(frame, SIGNAL(signalOpenUrlRequest(Akregator::OpenUrlRequest&)), this, SLOT(slotOpenUrlRequest(Akregator::OpenUrlRequest&)) );

    connect(frame, SIGNAL(signalCanGoBackToggled(Akregator::Frame*,bool)), this, SLOT(slotCanGoBackToggled(Akregator::Frame*,bool)) );
    connect(frame, SIGNAL(signalCanGoForwardToggled(Akregator::Frame*,bool)), this, SLOT(slotCanGoForwardToggled(Akregator::Frame*,bool)) );
    connect(frame, SIGNAL(signalIsReloadableToggled(Akregator::Frame*,bool)), this, SLOT(slotIsReloadableToggled(Akregator::Frame*,bool))
);

    connect(frame, SIGNAL(signalIsLoadingToggled(Akregator::Frame*,bool)), this, SLOT(slotIsLoadingToggled(Akregator::Frame*,bool)) );

    setPartGuiActive(frame->part(), false);

    emit signalFrameAdded(frame);

    if (m_frames.count() == 1)
        slotChangeFrame(frame->id());

}

void FrameManager::slotRemoveFrame(int id)
{
    Frame* frame = m_frames.value(id);

    if (!frame)
        return;
    if (!frame->isRemovable())
        return;
    frame->disconnect( this );

    if (m_currentFrame == frame)
    {
        slotChangeFrame(-1);
    }

    m_frames.insert(id, 0);
    m_frames.remove(id);
    emit signalFrameRemoved(id);
    delete frame;
}

Frame* FrameManager::findFrameById(int id) const
{
    return m_frames.value(id);
}

void FrameManager::slotChangeFrame(int frameId)
{
    Frame* frame = m_frames.value(frameId);
    if (frame == m_currentFrame)
        return;

    Frame* oldFrame = m_currentFrame;
    m_currentFrame = frame;

    if (oldFrame)
        setPartGuiActive(oldFrame->part(), false);

    if (frame)
    {
        setPartGuiActive(frame->part(), true);
        slotCanGoBackToggled(frame, frame->canGoBack());
        slotCanGoForwardToggled(frame, frame->canGoForward());
        slotIsReloadableToggled(frame, frame->isReloadable());
        slotIsLoadingToggled(frame, frame->isLoading());

        // TODO: handle removable flag

        switch (frame->state())
        {
            case Frame::Started:
                emit signalStarted();
                break;
            case Frame::Canceled:
                emit signalCanceled(QString());        //krazy:exclude=nullstrassign for old broken gcc
                break;
            case Frame::Idle:
            case Frame::Completed:
            default:
                emit signalCompleted();
        }

        emit signalCaptionChanged(frame->caption());
        emit signalTitleChanged(frame->title());
        emit signalLoadingProgress( frame->progress());
        emit signalStatusText( frame->statusText());
    }
    else
    {
        emit signalCompleted();
        emit signalCaptionChanged(QString());          //krazy:exclude=nullstrassign for old broken gcc
        emit signalTitleChanged(QString());            //krazy:exclude=nullstrassign for old broken gcc
        emit signalLoadingProgress(100);
        emit signalStatusText(QString());              //krazy:exclude=nullstrassign for old broken gcc
    }

    emit signalCurrentFrameChanged(oldFrame, frame);
}

void FrameManager::slotSetStarted(Frame* frame)
{
    if (frame == m_currentFrame)
        emit signalStarted();
}

void FrameManager::slotSetCanceled(Frame* frame, const QString& reason)
{
    if (frame == m_currentFrame)
        emit signalCanceled(reason);
}

void FrameManager::slotSetCompleted(Frame* frame)
{
    if (frame == m_currentFrame)
        emit signalCompleted();
}

void FrameManager::slotSetProgress(Frame* frame, int progress)
{
    if (frame == m_currentFrame)
        emit signalLoadingProgress(progress);
}

void FrameManager::slotSetIconChanged(Frame* frame, const QIcon& icon)
{
    if (frame != m_currentFrame)
        return;
    emit signalIconChanged(icon);
}

void FrameManager::slotSetCaption(Frame* frame, const QString& caption)
{
    if (frame == m_currentFrame)
        emit signalCaptionChanged(caption);
}

void FrameManager::slotSetTitle(Frame* frame, const QString& title)
{
    if (frame == m_currentFrame)
        emit signalTitleChanged(title);
}

void FrameManager::slotCanGoBackToggled(Frame* frame, bool enabled)
{
    if (frame == m_currentFrame)
        ActionManager::getInstance()->action("browser_back")->setEnabled(enabled);
}

void FrameManager::slotCanGoForwardToggled(Frame* frame, bool enabled)
{
    if (frame == m_currentFrame)
        ActionManager::getInstance()->action("browser_forward")->setEnabled(enabled);
}

void FrameManager::slotIsReloadableToggled(Frame* frame, bool enabled)
{
    if (frame == m_currentFrame)
        ActionManager::getInstance()->action("browser_reload")->setEnabled(enabled);
}

void FrameManager::slotIsLoadingToggled(Frame* frame, bool enabled)
{
    if (frame == m_currentFrame)
        ActionManager::getInstance()->action("browser_stop")->setEnabled(enabled);
}

void FrameManager::slotSetStatusText(Frame* frame, const QString& statusText)
{
    if (frame == m_currentFrame)
        emit signalStatusText(statusText);
}

void FrameManager::openUrl(OpenUrlRequest& request)
{
    if (request.browserArgs().newTab() || request.browserArgs().forcesNewWindow() || request.options() == OpenUrlRequest::NewTab)
    {
        int newFrameId = -1;
        emit signalRequestNewFrame(newFrameId);
        request.setFrameId(newFrameId);
    }

    if (m_frames.contains(request.frameId()))
    {
        Frame* frame = m_frames.value(request.frameId());
        if (frame->openUrl(request))
            request.setWasHandled(true);
        request.setPart(frame->part());
    }

    if (!request.openInBackground())
        emit signalSelectFrame(request.frameId());
}

void FrameManager::slotOpenUrlRequest(OpenUrlRequest& request )
{
    kDebug() <<"FrameManager::slotOpenUrlRequest():" << request.debugInfo();

    KUrl url = request.url();
    if (!url.isValid())
        return;

    if (!Settings::externalBrowserUseKdeDefault())
    {
        QHash<QChar,QString> map;
        map.insert(QLatin1Char('u'), url.url());
        const QString cmd = KMacroExpander::expandMacrosShellQuote(Settings::externalBrowserCustomCommand(), map);
        const QStringList args = KShell::splitArgs(cmd);
        if (!args.isEmpty())
        {
            KProcess::startDetached(args);
            return;
        }
    }

    if (request.args().mimeType().isEmpty())
        KToolInvocation::self()->invokeBrowser(url.url(), "0");
    else
        KRun::runUrl(url, request.args().mimeType(), 0 /*window*/, false, false);
}

void FrameManager::slotBrowserBackAboutToShow()
{
    if (m_currentFrame)
        m_currentFrame->slotHistoryBackAboutToShow();
}

void FrameManager::slotBrowserForwardAboutToShow()
{
    if (m_currentFrame)
        m_currentFrame->slotHistoryForwardAboutToShow();
}

void FrameManager::slotBrowserBack()
{
    if (m_currentFrame)
        m_currentFrame->slotHistoryBack();
}

void FrameManager::slotBrowserForward()
{
    if (m_currentFrame)
        m_currentFrame->slotHistoryForward();
}

void FrameManager::slotBrowserReload()
{
    if (m_currentFrame)
        m_currentFrame->slotReload();
}

void FrameManager::slotBrowserStop()
{
    if (m_currentFrame)
        m_currentFrame->slotStop();
}

void FrameManager::saveProperties(KConfigGroup & config)
{
    //write children
    QStringList strlst;
    QString newPrefix;
    QHash<int, Frame*>::const_iterator i;

    config.writeEntry( QString::fromLatin1( "Children" ), strlst );
    config.writeEntry( QString::fromLatin1( "activeChildIndex" ),
                       m_frames.key(m_currentFrame) );
}

} // namespace Akregator


