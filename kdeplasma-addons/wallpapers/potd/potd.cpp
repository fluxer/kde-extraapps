/*
 *   Copyright 2011 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "potd.h"

#include <QPainter>
#include <QThreadPool>

#include <KDebug>
#include <KAction>
#include <KFileDialog>
#include <KStandardAction>

static const QString DEFAULT_PROVIDER("apod");

SaveRunnable::SaveRunnable(Plasma::DataEngine *dataEngine, const QString &provider, const QString &path)
    : m_dataEngine(dataEngine),
      m_path(path)
{
    dataEngine->connectSource(provider, this);
    kDebug() << "saving to" << m_path;
    setAutoDelete(true);
}

void SaveRunnable::run()
{
    kDebug() << "saving?";
    if (m_image.isNull() || m_path.isEmpty()) {
        return;
    }

    kDebug() << "saving!";
    m_image.save(m_path);
}

void SaveRunnable::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (!m_dataEngine) {
        deleteLater();
        return;
    }

    m_image = data["Image"].value<QImage>();
    m_dataEngine.data()->disconnectSource(source, this);
    QThreadPool::globalInstance()->start(this);
}

PoTD::PoTD(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args)
{
    connect(this, SIGNAL(renderCompleted(QImage)), this, SLOT(wallpaperRendered(QImage)));
    dataEngine(QLatin1String("potd"))->connectSource(QLatin1String("Providers"), this);
    setUsingRenderingCache(false);

    QAction *saveFile = KStandardAction::save(this, SLOT(getSaveFileLocation()), this);
    saveFile->setText(i18n("Save wallpaper image..."));
    saveFile->setShortcut(QKeySequence());
    setContextualActions(QList<QAction *>() << saveFile);
}

void PoTD::init(const KConfigGroup &config)
{
    QString provider = config.readEntry(QLatin1String("provider"), DEFAULT_PROVIDER);
    if (provider.isEmpty() || (!m_providers.isEmpty() && !m_providers.contains(provider))) {
        provider = DEFAULT_PROVIDER;
    }

    if (provider != m_provider) {
        if (!m_provider.isEmpty()) {
            dataEngine(QLatin1String("potd"))->disconnectSource(m_provider, this);
        }

        m_provider = provider;

        if (!isPreviewing()) {
            dataEngine(QLatin1String("potd"))->connectSource(m_provider, this);
        }
    }
}

void PoTD::wallpaperRendered(const QImage &image)
{
    m_image = image;
    emit update(boundingRect());
}

void PoTD::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (source == QLatin1String("Providers")) {
        m_providers = data;
        if (!m_provider.isEmpty() && !m_providers.contains(m_provider)) {
            Plasma::DataEngine *engine = dataEngine(QLatin1String("potd"));
            engine->disconnectSource(m_provider, this);
            m_provider = DEFAULT_PROVIDER;
            engine->connectSource(m_provider, this);
        }
    } else if (source == m_provider) {
        QImage image = data["Image"].value<QImage>();
        render(image, boundingRect().size().toSize(), MaxpectResize);
    } else {
        dataEngine(QLatin1String("potd"))->disconnectSource(source, this);
    }
}

void PoTD::getSaveFileLocation()
{
    if (m_provider.isNull()) {
        return;
    }

    KFileDialog *fd = new KFileDialog(KUrl("kfiledialog:///frameplasmoid"), QString(), 0);
    fd->setOperationMode(KFileDialog::Saving);
    fd->setMode(KFile::LocalOnly);
    fd->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(fd, SIGNAL(okClicked()), this, SLOT(saveFile()));
    fd->show();
}

void PoTD::saveFile()
{
    KFileDialog *fd = qobject_cast<KFileDialog *>(sender());
    const QString &path = fd->selectedFile();
    if (path.isEmpty() || m_provider.isEmpty()) {
        return;
    }

    new SaveRunnable(dataEngine(QLatin1String("potd")), m_provider, path);
}

void PoTD::paint(QPainter *painter, const QRectF& exposedRect)
{
    if (m_image.isNull()) {
        painter->fillRect(exposedRect, QBrush(Qt::black));
        const QString provider = m_providers.isEmpty() || m_provider.isEmpty() ? QString()
                                                                               : m_providers.value(m_provider).toString();
        const QString text = provider.isEmpty() ? i18n("Loading the picture of the day...")
                                                : i18n("Loading the picture of the day from %1...", provider);
        QRect textRect = painter->fontMetrics().boundingRect(text);
        textRect.moveCenter(boundingRect().center().toPoint());
        painter->setPen(Qt::white);
        painter->drawText(textRect.topLeft(), text);
    } else {
        if (m_image.size() != boundingRect().size().toSize()) {
            Plasma::DataEngine *engine = dataEngine(QLatin1String("potd"));
            // refresh the data which will trigger a re-render
            engine->disconnectSource(m_provider, this);
            engine->connectSource(m_provider, this);
        }
        painter->drawImage(exposedRect, m_image, exposedRect);
    }
}

QWidget* PoTD::createConfigurationInterface(QWidget* parent)
{
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);
    m_configProvider.clear();

    Plasma::DataEngine::DataIterator it(m_providers);
    while (it.hasNext()) {
        it.next();
        m_ui.providers->addItem(it.value().toString(), it.key());
        if (m_provider == it.key()) {
            m_ui.providers->setCurrentIndex(m_ui.providers->count() - 1);
        }
    }
    connect(m_ui.providers, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsModified()));
    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));

    return widget;
}

void PoTD::save(KConfigGroup &config)
{
    if (m_configProvider.isEmpty()) {
        config.writeEntry("provider", m_provider);
    } else {
        config.writeEntry("provider", m_configProvider);
        m_configProvider.clear();
    }
}

void PoTD::settingsModified()
{
    m_configProvider = m_ui.providers->itemData(m_ui.providers->currentIndex()).toString();
    emit settingsChanged(true);
}

#include "potd.moc"
