/**
 * Copyright (C) 2002-2004 Scott Wheeler <wheeler@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "statuslabel.h"

#include <kaction.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <ksqueezedtextlabel.h>
#include <klocale.h>
#include <kdebug.h>

#include <QMouseEvent>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QEvent>

#include "filehandle.h"
#include "playlistinterface.h"
#include "actioncollection.h"
#include "tag.h"

using namespace ActionCollection;

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

StatusLabel::StatusLabel(PlaylistInterface *playlist, QWidget *parent) :
    KHBox(parent),
    PlaylistObserver(playlist),
    m_showTimeRemaining(false)
{
    QFrame *trackAndPlaylist = new QFrame(this);
    trackAndPlaylist->setFrameStyle(Box | Sunken);
    trackAndPlaylist->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Make sure that we have enough of a margin to suffice for the borders,
    // hence the "lineWidth() * 2"
    QHBoxLayout *trackAndPlaylistLayout = new QHBoxLayout( trackAndPlaylist );
    trackAndPlaylistLayout->setMargin( trackAndPlaylist->lineWidth() * 2 );
    trackAndPlaylistLayout->setSpacing( 5 );
    trackAndPlaylistLayout->setObjectName( QLatin1String( "trackAndPlaylistLayout" ));
    trackAndPlaylistLayout->addSpacing(5);

    m_playlistLabel = new KSqueezedTextLabel(trackAndPlaylist);
    trackAndPlaylistLayout->addWidget(m_playlistLabel);
    m_playlistLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_playlistLabel->setTextFormat(Qt::PlainText);
    m_playlistLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_trackLabel = new KSqueezedTextLabel(trackAndPlaylist);
    trackAndPlaylistLayout->addWidget(m_trackLabel);
    m_trackLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_trackLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_trackLabel->setTextFormat(Qt::PlainText);

    trackAndPlaylistLayout->addSpacing(5);

    m_itemTimeLabel = new QLabel(this);
    QFontMetrics fontMetrics(font());
    m_itemTimeLabel->setAlignment(Qt::AlignCenter);
    m_itemTimeLabel->setMinimumWidth(fontMetrics.boundingRect("000:00 / 000:00").width());
    m_itemTimeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_itemTimeLabel->setFrameStyle(Box | Sunken);
    m_itemTimeLabel->installEventFilter(this);

    setItemTotalTime(0);
    setItemCurrentTime(0);

    KHBox *jumpBox = new KHBox(this);
    jumpBox->setFrameStyle(Box | Sunken);
    jumpBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    QPushButton *jumpButton = new QPushButton(jumpBox);
    jumpButton->setIcon(SmallIcon("go-up"));
    jumpButton->setFlat(true);

    jumpButton->setToolTip( i18n("Jump to the currently playing item"));
    connect(jumpButton, SIGNAL(clicked()), action("showPlaying"), SLOT(trigger()));

    installEventFilter(this);

    updateData();
}

StatusLabel::~StatusLabel()
{

}

void StatusLabel::updateCurrent()
{
    if(playlist()->playing()) {
        FileHandle file = playlist()->currentFile();

        QString mid =  file.tag()->artist().isEmpty() || file.tag()->title().isEmpty()
            ? QString::null : QString(" - ");	//krazy:exclude=nullstrassign for old broken gcc

        QString text = file.tag()->artist() + mid + file.tag()->title();

        m_trackLabel->setText(text);
        m_playlistLabel->setText(playlist()->name().simplified());
    }
}

void StatusLabel::updateData()
{
    updateCurrent();

    if(!playlist()->playing()) {
        setItemTotalTime(0);
        setItemCurrentTime(0);

        int time = playlist()->time();

        int days = time / (60 * 60 * 24);
        int hours = time / (60 * 60) % 24;
        int minutes = time / 60 % 60;
        int seconds = time % 60;

        QString timeString;

        if(days > 0) {
            timeString = i18np("1 day", "%1 days", days);
            timeString.append(" ");
        }

        if(days > 0 || hours > 0)
            timeString.append(QString().sprintf("%1d:%02d:%02d", hours, minutes, seconds));
        else
            timeString.append(QString().sprintf("%1d:%02d", minutes, seconds));

        m_playlistLabel->setText(playlist()->name());
        m_trackLabel->setText(i18np("1 item", "%1 items", playlist()->count()) + " - " + timeString);
    }
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void StatusLabel::updateTime()
{
    int minutes;
    int seconds;

    if(m_showTimeRemaining) {
        minutes = int((m_itemTotalTime - m_itemCurrentTime) / 60);
        seconds = (m_itemTotalTime - m_itemCurrentTime) % 60;
    }
    else {
        minutes = int(m_itemCurrentTime / 60);
        seconds = m_itemCurrentTime % 60;
    }

    int totalMinutes = int(m_itemTotalTime / 60);
    int totalSeconds = m_itemTotalTime % 60;

    QString timeString = formatTime(minutes, seconds) +  " / " +
        formatTime(totalMinutes, totalSeconds);
    m_itemTimeLabel->setText(timeString);
}

bool StatusLabel::eventFilter(QObject *o, QEvent *e)
{
    if(!o || !e)
        return false;

    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
    if(e->type() == QEvent::MouseButtonRelease &&
       mouseEvent->button() == Qt::LeftButton)
    {
        if(o == m_itemTimeLabel) {
            m_showTimeRemaining = !m_showTimeRemaining;
            updateTime();
        }
        else
            action("showPlaying")->trigger();

        return true;
    }
    return false;
}

QString StatusLabel::formatTime(int minutes, int seconds) // static
{
    QString m = QString::number(minutes);
    if(m.length() == 1)
        m = '0' + m;
    QString s = QString::number(seconds);
    if(s.length() == 1)
        s = '0' + s;
    return m + ':' + s;
}

#include "statuslabel.moc"

// vim: set et sw=4 tw=0 sta:
