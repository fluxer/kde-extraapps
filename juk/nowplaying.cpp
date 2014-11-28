/**
 * Copyright (C) 2004 Scott Wheeler <wheeler@kde.org>
 * Copyright (C) 2009 Michael Pyne <mpyne@kde.org>
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

#include "nowplaying.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <krandom.h>
#include <kglobalsettings.h>
#include <kio/netaccess.h>

#include <QImage>
#include <QLayout>
#include <QEvent>
#include <QDrag>
#include <QTimer>
#include <QPoint>
#include <QFrame>
#include <QDropEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QUrl>
#include <QList>
#include <QTextDocument>

#include "playlistcollection.h"
#include "playlistitem.h"
#include "coverinfo.h"
#include "covermanager.h"
#include "tag.h"
#include "collectionlist.h"

static const int imageSize = 64;

struct Line : public QFrame
{
    Line(QWidget *parent) : QFrame(parent) { setFrameShape(VLine); }
};

////////////////////////////////////////////////////////////////////////////////
// NowPlaying
////////////////////////////////////////////////////////////////////////////////

NowPlaying::NowPlaying(QWidget *parent, PlaylistCollection *collection) :
    QWidget(parent),
    m_observer(this, collection),
    // Also watch the collection
    m_collectionListObserver(this, CollectionList::instance()),
    m_collection(collection)
{
    setObjectName(QLatin1String("NowPlaying"));

    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);

    layout->setMargin(0);
    layout->setSpacing(3);
    setFixedHeight(imageSize + 2);

    layout->addWidget(new CoverItem(this), 0);
    layout->addWidget(new TrackItem(this), 2);
    layout->addWidget(new Line(this), 0);

    hide();
}

void NowPlaying::addItem(NowPlayingItem *item)
{
    m_items.append(item);
}

PlaylistCollection *NowPlaying::collection() const
{
    return m_collection;
}

void NowPlaying::slotUpdate(const FileHandle &file)
{
    m_file = file;
    if(file.isNull()) {
        hide();
        emit nowPlayingHidden();
        return;
    }
    else
        show();

    foreach(NowPlayingItem *item, m_items)
        item->update(file);
}

void NowPlaying::slotReloadCurrentItem()
{
    foreach(NowPlayingItem *item, m_items)
        item->update(m_file);
}

////////////////////////////////////////////////////////////////////////////////
// CoverItem
////////////////////////////////////////////////////////////////////////////////

CoverItem::CoverItem(NowPlaying *parent) :
    QLabel(parent),
    NowPlayingItem(parent)
{
    setObjectName( QLatin1String("CoverItem" ));
    setFixedHeight(parent->height() - parent->layout()->margin() * 2);
    setMargin(1);
    setAcceptDrops(true);
}

void CoverItem::update(const FileHandle &file)
{
    m_file = file;

    if(!file.isNull() && file.coverInfo()->hasCover()) {
        show();
        setPixmap(
            file.coverInfo()->pixmap(CoverInfo::Thumbnail)
            .scaled(imageSize, imageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else
        hide();
}

void CoverItem::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_dragging) {
        m_dragging = false;
        return;
    }

    if(event->x() >= 0 && event->y() >= 0 &&
       event->x() < width() && event->y() < height() &&
       event->button() == Qt::LeftButton &&
       m_file.coverInfo()->hasCover())
    {
        m_file.coverInfo()->popup();
    }

    QLabel::mousePressEvent(event);
}

void CoverItem::mousePressEvent(QMouseEvent *e)
{
    m_dragging = false;
    m_dragStart = e->globalPos();
}

void CoverItem::mouseMoveEvent(QMouseEvent *e)
{
    if(m_dragging)
        return;

    QPoint diff = m_dragStart - e->globalPos();
    if(diff.manhattanLength() > KGlobalSettings::dndEventDelay()) {

        // Start a drag.

        m_dragging = true;

        QDrag *drag = new QDrag(this);
        CoverDrag *data = new CoverDrag(m_file.coverInfo()->coverId());

        drag->setMimeData(data);
        drag->exec(Qt::CopyAction);
    }
}

void CoverItem::dragEnterEvent(QDragEnterEvent *e)
{
    e->setAccepted(CoverDrag::isCover(e->mimeData()) || e->mimeData()->hasUrls());
}

void CoverItem::dropEvent(QDropEvent *e)
{
    QImage image;
    QList<QUrl> urls;
    coverKey key;

    if(e->source() == this)
        return;

    key = CoverDrag::idFromData(e->mimeData());
    if(key != CoverManager::NoMatch) {
        m_file.coverInfo()->setCoverId(key);
        update(m_file);
    }
    else if(e->mimeData()->hasImage()) {
        m_file.coverInfo()->setCover(qvariant_cast<QImage>(e->mimeData()->imageData()));
        update(m_file);
    }
    else {
        urls = e->mimeData()->urls();
        if(urls.isEmpty())
            return;

        QString fileName;

        if(KIO::NetAccess::download(urls.front(), fileName, this)) {
            if(image.load(fileName)) {
                m_file.coverInfo()->setCover(image);
                update(m_file);
            }
            else
                kError() << "Unable to load image from " << urls.front() << endl;

            KIO::NetAccess::removeTempFile(fileName);
        }
        else
            kError() << "Unable to download " << urls.front() << endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
// TrackItem
////////////////////////////////////////////////////////////////////////////////

TrackItem::TrackItem(NowPlaying *parent) :
    QWidget(parent),
    NowPlayingItem(parent)
{
    setObjectName( QLatin1String("TrackItem" ));
    setFixedHeight(parent->height() - parent->layout()->margin() * 2);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_label = new QLabel(this);
    m_label->setWordWrap(true);

    m_label->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::LinksAccessibleByKeyboard);

    layout->addStretch();
    layout->addWidget(m_label);
    layout->addStretch();

    connect(m_label, SIGNAL(linkActivated(QString)), this,
            SLOT(slotOpenLink(QString)));

    // Ensure that if we're filtering results, that the filtering is cleared if we
    // hide the now playing bar so that the user can select tracks normally.

    connect(parent, SIGNAL(nowPlayingHidden()), SLOT(slotClearShowMore()));
}

void TrackItem::update(const FileHandle &file)
{
    m_file = file;
    QTimer::singleShot(0, this, SLOT(slotUpdate()));
}

void TrackItem::slotOpenLink(const QString &link)
{
    PlaylistCollection *collection = NowPlayingItem::parent()->collection();

    if(link == "artist")
        collection->showMore(m_file.tag()->artist());
    else if(link == "album")
        collection->showMore(m_file.tag()->artist(), m_file.tag()->album());
    else if(link == "clear")
        collection->clearShowMore();

    update(m_file);
}

void TrackItem::slotUpdate()
{
    if(m_file.isNull()) {
        m_label->setText(QString());
        return;
    }

    QString title  = Qt::escape(m_file.tag()->title());
    QString artist = Qt::escape(m_file.tag()->artist());
    QString album  = Qt::escape(m_file.tag()->album());
    QString separator = (artist.isNull() || album.isNull()) ? QString::null : QString(" - ");	//krazy:exclude=nullstrassign for old broken gcc

    // This block-o-nastiness makes the font smaller and smaller until it actually fits.

    int size = 4;
    QString format =
        "<font size=\"+%1\"><b>%2</b></font>"
        "<br />"
        "<font size=\"+%3\"><b><a href=\"artist\">%4</a>%5<a href=\"album\">%6</a></b>";

    if(NowPlayingItem::parent()->collection()->showMoreActive())
        format.append(QString(" (<a href=\"clear\">%1</a>)").arg(i18n("back to playlist")));

    format.append("</font>");

    do {
        m_label->setText(format.arg(size).arg(title).arg(size - 2)
                         .arg(artist).arg(separator).arg(album));
        --size;
    } while(m_label->heightForWidth(m_label->width()) > imageSize && size >= 0);

    m_label->setFixedHeight(qMin(imageSize, m_label->heightForWidth(m_label->width())));
}

void TrackItem::slotClearShowMore()
{
    PlaylistCollection *collection = NowPlayingItem::parent()->collection();
    Q_ASSERT(collection);
    collection->clearShowMore();
}

#include "nowplaying.moc"

// vim: set et sw=4 tw=0 sta:
