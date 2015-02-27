/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *   Copyright (C) 2008-2010 Matthias Fuchs <mat69@gmx.net>                *
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

#include "configwidget.h"
#include "comicmodel.h"

#include <QtCore/QTimer>
#include <QtGui/QSortFilterProxyModel>

#include <KConfigDialog>

ComicUpdater::ComicUpdater( QObject *parent )
  : QObject( parent ),
    mUpdateIntervall( 3 ),
    m_updateTimer( 0 )
{
}

ComicUpdater::~ComicUpdater()
{
}

void ComicUpdater::init(const KConfigGroup &group)
{
    mGroup = group;
}

void ComicUpdater::load()
{
    //check when the last update happened and update if necessary
    mUpdateIntervall = mGroup.readEntry( "updateIntervall", 3 );
}

void ComicUpdater::save()
{
    mGroup.writeEntry( "updateIntervall", mUpdateIntervall );
}

void ComicUpdater::applyConfig( ConfigWidget *widget )
{
    mUpdateIntervall = widget->updateIntervall();
}

ConfigWidget::ConfigWidget( Plasma::DataEngine *engine, ComicModel *model, QSortFilterProxyModel *proxy, KConfigDialog *parent )
    : QWidget( parent ), mEngine( engine ), mModel( model ), mProxyModel( proxy )
{
    comicSettings = new QWidget( this );
    comicUi.setupUi( comicSettings );
    comicUi.pushButton_GHNS->setIcon( KIcon( "get-hot-new-stuff" ) );

    appearanceSettings = new QWidget();
    appearanceUi.setupUi( appearanceSettings );

    advancedSettings = new QWidget();
    advancedUi.setupUi( advancedSettings );

    connect( comicUi.pushButton_GHNS, SIGNAL(clicked()), this, SLOT(getNewStuff()) );

    comicUi.listView_comic->setModel( mProxyModel );
    comicUi.listView_comic->resizeColumnToContents( 0 );

    // "Apply" button connections
    connect(comicUi.listView_comic , SIGNAL(clicked(QModelIndex)), this , SIGNAL(enableApply()));
    connect(comicUi.pushButton_GHNS , SIGNAL(clicked(bool)), this , SIGNAL(enableApply()));
    connect(comicUi.checkBox_middle , SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(comicUi.updateIntervall, SIGNAL(valueChanged(int)), this, SIGNAL(enableApply()));
    connect(comicUi.updateIntervallComicStrips, SIGNAL(valueChanged(int)), this, SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_arrows, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_title, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_identifier, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_author, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.checkBox_url, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));
    connect(appearanceUi.kbuttongroup, SIGNAL(changed(int)), this , SIGNAL(enableApply()));
    connect(advancedUi.maxComicLimit, SIGNAL(valueChanged(int)), this, SIGNAL(enableApply()));
    connect(advancedUi.errorPicture, SIGNAL(toggled(bool)), this , SIGNAL(enableApply()));

    mEngine->connectSource( QLatin1String( "providers" ), this );
}

ConfigWidget::~ConfigWidget()
{
    mEngine->disconnectSource( QLatin1String( "providers" ), this );
}

void ConfigWidget::getNewStuff()
{
}

void ConfigWidget::dataUpdated(const QString &name, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(name);
    mModel->setComics( data, mModel->selected() );
    comicUi.listView_comic->resizeColumnToContents( 0 );
}

void ConfigWidget::setShowComicUrl( bool show )
{
    appearanceUi.checkBox_url->setChecked( show );
}

bool ConfigWidget::showComicUrl() const
{
    return appearanceUi.checkBox_url->isChecked();
}

void ConfigWidget::setShowComicAuthor( bool show )
{
    appearanceUi.checkBox_author->setChecked( show );
}

bool ConfigWidget::showComicAuthor() const
{
    return appearanceUi.checkBox_author->isChecked();
}

void ConfigWidget::setShowComicTitle( bool show )
{
    appearanceUi.checkBox_title->setChecked( show );
}

bool ConfigWidget::showComicTitle() const
{
    return appearanceUi.checkBox_title->isChecked();
}

void ConfigWidget::setShowComicIdentifier( bool show )
{
    appearanceUi.checkBox_identifier->setChecked( show );
}

bool ConfigWidget::showComicIdentifier() const
{
    return appearanceUi.checkBox_identifier->isChecked();
}

void ConfigWidget::setShowErrorPicture( bool show )
{
    advancedUi.errorPicture->setChecked( show );
}

bool ConfigWidget::showErrorPicture() const
{
    return advancedUi.errorPicture->isChecked();
}


void ConfigWidget::setArrowsOnHover( bool arrows )
{
    return appearanceUi.checkBox_arrows->setChecked( arrows );
}

bool ConfigWidget::arrowsOnHover() const
{
    return appearanceUi.checkBox_arrows->isChecked();
}

void ConfigWidget::setMiddleClick( bool checked )
{
    comicUi.checkBox_middle->setChecked( checked );
}

bool ConfigWidget::middleClick() const
{
    return comicUi.checkBox_middle->isChecked();
}

void ConfigWidget::setTabView(int tabView)
{
    appearanceUi.kbuttongroup->setSelected( tabView );
}

int ConfigWidget::tabView() const
{
    return appearanceUi.kbuttongroup->selected();
}

int ConfigWidget::maxComicLimit() const
{
    return advancedUi.maxComicLimit->value();
}

void ConfigWidget::setMaxComicLimit( int limit )
{
    advancedUi.maxComicLimit->setValue( limit );
}

void ConfigWidget::setUpdateIntervall( int days )
{
    comicUi.updateIntervall->setValue( days );
}

int ConfigWidget::updateIntervall() const
{
    return comicUi.updateIntervall->value();
}

void ConfigWidget::setCheckNewComicStripsIntervall( int minutes )
{
    comicUi.updateIntervallComicStrips->setValue( minutes );
}

int ConfigWidget::checkNewComicStripsIntervall() const
{
    return comicUi.updateIntervallComicStrips->value();
}

#include "moc_configwidget.cpp"
