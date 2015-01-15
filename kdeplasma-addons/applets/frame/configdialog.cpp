/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "configdialog.h"

#include <QThreadPool>

#include <KLocale>
#include <KStandardDirs>

#include "picture.h"
#include "imagescaler.h"

ConfigDialog::ConfigDialog(QWidget *parent)
        : QObject(parent)
{
    m_picture = new Picture(this);
    connect(m_picture, SIGNAL(pictureLoaded(QImage)), this, SLOT(pictureLoaded(QImage)));

    appearanceSettings = new QWidget();
    appearanceUi.setupUi(appearanceSettings);

    imageSettings = new QWidget();
    imageUi.setupUi(imageSettings);

    imageUi.addDirButton->setIcon(KIcon("list-add"));
    imageUi.removeDirButton->setIcon(KIcon("list-remove"));
    imageUi.slideShowDelay->setMinimumTime(QTime(0, 0, 1)); // minimum to 1 second

    QString monitorPath = KStandardDirs::locate("data",  "kcontrol/pics/monitor.png");
    // Size of monitor image: 200x186
    // Geometry of "display" part of monitor image: (23,14)-[151x115]
    imageUi.monitorLabel->setPixmap(QPixmap(monitorPath));
    imageUi.monitorLabel->setWhatsThis(i18n(
                                           "This picture of a monitor contains a preview of "
                                           "the picture you currently have in your frame."));
    m_preview = new QLabel(imageUi.monitorLabel);
    m_preview->setScaledContents(true);
    m_preview->setGeometry(23, 14, 151, 115);
    m_preview->show();

    connect(imageUi.picRequester, SIGNAL(urlSelected(KUrl)), this, SLOT(changePreview(KUrl)));
    connect(imageUi.picRequester->comboBox(), SIGNAL(activated(QString)), this, SLOT(changePreview(QString)));
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::setRoundCorners(bool round)
{
    appearanceUi.roundCheckBox->setChecked(round);
}

bool ConfigDialog::roundCorners() const
{
    return appearanceUi.roundCheckBox->isChecked();
}

void ConfigDialog::setRandom(bool random)
{
    imageUi.randomCheckBox->setChecked(random);
}

bool ConfigDialog::random() const
{
    return imageUi.randomCheckBox->isChecked();
}

void ConfigDialog::setShadow(bool shadow)
{
    appearanceUi.shadowCheckBox->setChecked(shadow);
}

bool ConfigDialog::shadow() const
{
    return appearanceUi.shadowCheckBox->isChecked();
}

void ConfigDialog::setShowFrame(bool show)
{
    appearanceUi.frameCheckBox->setChecked(show);
}

bool ConfigDialog::showFrame() const
{
    return appearanceUi.frameCheckBox->isChecked();
}

void ConfigDialog::setFrameColor(const QColor& frameColor)
{
    appearanceUi.changeFrameColor->setColor(frameColor);
}

QColor ConfigDialog::frameColor() const
{
    return appearanceUi.changeFrameColor->color();
}

void ConfigDialog::setCurrentUrl(const KUrl& currentUrl)
{
    imageUi.picRequester->setUrl(currentUrl);
}

KUrl ConfigDialog::currentUrl() const
{
    return imageUi.picRequester->url();
}

void ConfigDialog::previewPicture(const QImage &image)
{
    ImageScaler *scaler = new ImageScaler(image, QRect(23, 14, 151, 115).size());
    connect(scaler, SIGNAL(scaled(QImage)), this, SLOT(previewScaled(QImage)));
    QThreadPool::globalInstance()->start(scaler);
}

void ConfigDialog::previewScaled(const QImage &image)
{
    m_preview->setPixmap(QPixmap::fromImage(image));
}

void ConfigDialog::changePreview(const KUrl &path)
{
    m_picture->setPicture(path);
}

void ConfigDialog::pictureLoaded(QImage image)
{
    previewPicture(image);
}

void ConfigDialog::changePreview(const QString &path)
{
    m_picture->setPicture(KUrl(path));
}
