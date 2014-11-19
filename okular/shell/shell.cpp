/***************************************************************************
 *   Copyright (C) 2002 by Wilco Greven <greven@kde.org>                   *
 *   Copyright (C) 2002 by Chris Cheney <ccheney@cheney.cx>                *
 *   Copyright (C) 2003 by Benjamin Meyer <benjamin@csh.rit.edu>           *
 *   Copyright (C) 2003-2004 by Christophe Devriese                        *
 *                         <Christophe.Devriese@student.kuleuven.ac.be>    *
 *   Copyright (C) 2003 by Laurent Montel <montel@kde.org>                 *
 *   Copyright (C) 2003-2004 by Albert Astals Cid <aacid@kde.org>          *
 *   Copyright (C) 2003 by Luboš Luňák <l.lunak@kde.org>                   *
 *   Copyright (C) 2003 by Malcolm Hunter <malcolm.hunter@gmx.co.uk>       *
 *   Copyright (C) 2004 by Dominique Devriese <devriese@kde.org>           *
 *   Copyright (C) 2004 by Dirk Mueller <mueller@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "shell.h"

// qt/kde includes
#include <qdesktopwidget.h>
#include <qtimer.h>
#include <QtDBus/qdbusconnection.h>
#include <kaction.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kpluginloader.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kstandardaction.h>
#include <ktoolbar.h>
#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kio/netaccess.h>
#include <krecentfilesaction.h>
#include <kservicetypetrader.h>
#include <ktoggleaction.h>
#include <ktogglefullscreenaction.h>
#include <kactioncollection.h>
#include <kwindowsystem.h>
#include <ktabwidget.h>
#include <kxmlguifactory.h>
#include <QDragMoveEvent>
#include <QTabBar>

#ifdef KActivities_FOUND
#include <KActivities/ResourceInstance>
#endif

// local includes
#include "kdocumentviewer.h"
#include "../interfaces/viewerinterface.h"
#include "shellutils.h"

static const char *shouldShowMenuBarComingFromFullScreen = "shouldShowMenuBarComingFromFullScreen";
static const char *shouldShowToolBarComingFromFullScreen = "shouldShowToolBarComingFromFullScreen";

Shell::Shell( const QString &serializedOptions )
  : KParts::MainWindow(), m_menuBarWasShown(true), m_toolBarWasShown(true)
#ifdef KActivities_FOUND
    , m_activityResource(0)
#endif
    , m_isValid(true)
{
  setObjectName( QLatin1String( "okular::Shell" ) );
  setContextMenuPolicy( Qt::NoContextMenu );
  // set the shell's ui resource file
  setXMLFile("shell.rc");
  m_fileformatsscanned = false;
  m_showMenuBarAction = 0;
  // this routine will find and load our Part.  it finds the Part by
  // name which is a bad idea usually.. but it's alright in this
  // case since our Part is made for this Shell
  m_partFactory = KPluginLoader("okularpart").factory();
  if (!m_partFactory)
  {
    // if we couldn't find our Part, we exit since the Shell by
    // itself can't do anything useful
    m_isValid = false;
    KMessageBox::error(this, i18n("Unable to find the Okular component."));
    return;
  }

  // now that the Part plugin is loaded, create the part
  KParts::ReadWritePart* const firstPart = m_partFactory->create< KParts::ReadWritePart >( this );
  if (firstPart)
  {
    // Setup tab bar
    m_tabWidget = new KTabWidget( this );
    m_tabWidget->setTabsClosable( true );
    m_tabWidget->setElideMode( Qt::ElideRight );
    m_tabWidget->setTabBarHidden( true );
    m_tabWidget->setDocumentMode( true );
    m_tabWidget->setMovable( true );
    connect( m_tabWidget, SIGNAL(currentChanged(int)), SLOT(setActiveTab(int)) );
    connect( m_tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)) );
    connect( m_tabWidget, SIGNAL(testCanDecode(const QDragMoveEvent*,bool&)), SLOT(testTabDrop(const QDragMoveEvent*,bool&)) );
    connect( m_tabWidget, SIGNAL(receivedDropEvent(QDropEvent*)), SLOT(handleTabDrop(QDropEvent*)) );
    connect( m_tabWidget->tabBar(), SIGNAL(tabMoved(int,int)), SLOT(moveTabData(int,int)) );

    setCentralWidget( m_tabWidget );

    // then, setup our actions
    setupActions();
    // and integrate the part's GUI with the shell's
    setupGUI(Keys | ToolBar | Save);
    createGUI(firstPart);
    connectPart( firstPart );

    m_tabs.append( firstPart );
    m_tabWidget->addTab( firstPart->widget(), QString() );

    readSettings();

    m_unique = ShellUtils::unique(serializedOptions);
    if (m_unique)
    {
        m_unique = QDBusConnection::sessionBus().registerService("org.kde.okular");
        if (!m_unique)
            KMessageBox::information(this, i18n("There is already a unique Okular instance running. This instance won't be the unique one."));
    }
    if (ShellUtils::noRaise(serializedOptions))
    {
        setAttribute(Qt::WA_ShowWithoutActivating);
    }
    
    QDBusConnection::sessionBus().registerObject("/okularshell", this, QDBusConnection::ExportScriptableSlots);
  }
  else
  {
    m_isValid = false;
    KMessageBox::error(this, i18n("Unable to find the Okular component."));
  }
}

bool Shell::isValid() const
{
    return m_isValid;
}

void Shell::showOpenRecentMenu()
{
    m_recent->menu()->popup(QCursor::pos());
}

Shell::~Shell()
{
    if( !m_tabs.empty() )
    {
        writeSettings();
        for( QList<TabState>::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it )
        {
           it->part->closeUrl( false );
        }
    }
    if (m_unique)
        QDBusConnection::sessionBus().unregisterService("org.kde.okular");
}

// Open a new document if we have space for it
// This can hang if called on a unique instance and openUrl pops a messageBox
bool Shell::openDocument( const QString& url, const QString &serializedOptions )
{
    if( m_tabs.size() <= 0 )
       return false;

    KParts::ReadWritePart* const part = m_tabs[0].part;

    // Return false if we can't open new tabs and the only part is occupied
    if ( !dynamic_cast<Okular::ViewerInterface*>(part)->openNewFilesInTabs()
         && !part->url().isEmpty()
         && !ShellUtils::unique(serializedOptions))
    {
        return false;
    }

    openUrl( url, serializedOptions );

    return true;
}

bool Shell::canOpenDocs( int numDocs, int desktop )
{
   if( m_tabs.size() <= 0 || numDocs <= 0 || m_unique )
      return false;

   KParts::ReadWritePart* const part = m_tabs[0].part;
   const bool allowTabs = dynamic_cast<Okular::ViewerInterface*>(part)->openNewFilesInTabs();

   if( !allowTabs && (numDocs > 1 || !part->url().isEmpty()) )
      return false;

   const KWindowInfo winfo( window()->effectiveWinId(), KWindowSystem::WMDesktop );
   if( winfo.desktop() != desktop )
      return false;

   return true;
}

void Shell::openUrl( const KUrl & url, const QString &serializedOptions )
{
    const int activeTab = m_tabWidget->currentIndex();
    if ( activeTab < m_tabs.size() )
    {
        KParts::ReadWritePart* const activePart = m_tabs[activeTab].part;
        if( !activePart->url().isEmpty() )
        {
            if( m_unique )
            {
                applyOptionsToPart( activePart, serializedOptions );
                activePart->openUrl( url );
            }
            else
            {
                if( dynamic_cast<Okular::ViewerInterface *>(activePart)->openNewFilesInTabs() )
                {
                    openNewTab( url, serializedOptions );
                }
                else
                {
                    Shell* newShell = new Shell( serializedOptions );
                    newShell->openUrl( url, serializedOptions );
                    newShell->show();
                }
            }
        }
        else
        {
            m_tabWidget->setTabText( activeTab, url.fileName() );
            applyOptionsToPart( activePart, serializedOptions );
            bool openOk = activePart->openUrl( url );
            const bool isstdin = url.fileName( KUrl::ObeyTrailingSlash ) == QLatin1String( "-" );
            if ( !isstdin )
            {
                if ( openOk )
                {
#ifdef KActivities_FOUND
                    if ( !m_activityResource )
                        m_activityResource = new KActivities::ResourceInstance( window()->winId(), this );

                    m_activityResource->setUri( url );
#endif
                    m_recent->addUrl( url );
                }
                else
                    m_recent->removeUrl( url );
            }
        }
    }
}

void Shell::closeUrl()
{
    closeTab( m_tabWidget->currentIndex() );
}

void Shell::readSettings()
{
    m_recent->loadEntries( KGlobal::config()->group( "Recent Files" ) );
    m_recent->setEnabled( true ); // force enabling

    const KConfigGroup group = KGlobal::config()->group( "Desktop Entry" );
    bool fullScreen = group.readEntry( "FullScreen", false );
    setFullScreen( fullScreen );
    
    if (fullScreen)
    {
        m_menuBarWasShown = group.readEntry( shouldShowMenuBarComingFromFullScreen, true );
        m_toolBarWasShown = group.readEntry( shouldShowToolBarComingFromFullScreen, true );
    }
}

void Shell::writeSettings()
{
    m_recent->saveEntries( KGlobal::config()->group( "Recent Files" ) );
    KConfigGroup group = KGlobal::config()->group( "Desktop Entry" );
    group.writeEntry( "FullScreen", m_fullScreenAction->isChecked() );
    if (m_fullScreenAction->isChecked())
    {
        group.writeEntry( shouldShowMenuBarComingFromFullScreen, m_menuBarWasShown );
        group.writeEntry( shouldShowToolBarComingFromFullScreen, m_toolBarWasShown );
    }
    KGlobal::config()->sync();
}

void Shell::setupActions()
{
  KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
  m_recent = KStandardAction::openRecent( this, SLOT(openUrl(KUrl)), actionCollection() );
  m_recent->setToolBarMode( KRecentFilesAction::MenuMode );
  connect( m_recent, SIGNAL(triggered()), this, SLOT(showOpenRecentMenu()) );
  m_recent->setToolTip( i18n("Click to open a file\nClick and hold to open a recent file") );
  m_recent->setWhatsThis( i18n( "<b>Click</b> to open a file or <b>Click and hold</b> to select a recent file" ) );
  m_printAction = KStandardAction::print( this, SLOT(print()), actionCollection() );
  m_printAction->setEnabled( false );
  m_closeAction = KStandardAction::close( this, SLOT(closeUrl()), actionCollection() );
  m_closeAction->setEnabled( false );
  KStandardAction::quit(this, SLOT(close()), actionCollection());

  setStandardToolBarMenuEnabled(true);

  m_showMenuBarAction = KStandardAction::showMenubar( this, SLOT(slotShowMenubar()), actionCollection());
  m_fullScreenAction = KStandardAction::fullScreen( this, SLOT(slotUpdateFullScreen()), this,actionCollection() );

  m_nextTabAction = actionCollection()->addAction("tab-next");
  m_nextTabAction->setText( i18n("Next Tab") );
  m_nextTabAction->setShortcut( KStandardShortcut::tabNext() );
  m_nextTabAction->setEnabled( false );
  connect( m_nextTabAction, SIGNAL(triggered()), this, SLOT(activateNextTab()) );

  m_prevTabAction = actionCollection()->addAction("tab-previous");
  m_prevTabAction->setText( i18n("Previous Tab") );
  m_prevTabAction->setShortcut( KStandardShortcut::tabPrev() );
  m_prevTabAction->setEnabled( false );
  connect( m_prevTabAction, SIGNAL(triggered()), this, SLOT(activatePrevTab()) );
}

void Shell::saveProperties(KConfigGroup &group)
{
  // the 'config' object points to the session managed
  // config file.  anything you write here will be available
  // later when this app is restored
    emit saveDocumentRestoreInfo(group);
}

void Shell::readProperties(const KConfigGroup &group)
{
  // the 'config' object points to the session managed
  // config file.  this function is automatically called whenever
  // the app is being restored.  read in here whatever you wrote
  // in 'saveProperties'
    emit restoreDocument(group);
}

QStringList Shell::fileFormats() const
{
    QStringList supportedPatterns;

    QString constraint( "(Library == 'okularpart')" );
    QLatin1String basePartService( "KParts/ReadOnlyPart" );
    KService::List offers = KServiceTypeTrader::self()->query( basePartService, constraint );
    KService::List::ConstIterator it = offers.constBegin(), itEnd = offers.constEnd();
    for ( ; it != itEnd; ++it )
    {
        KService::Ptr service = *it;
        QStringList mimeTypes = service->serviceTypes();
        foreach ( const QString& mimeType, mimeTypes )
            if ( mimeType != basePartService )
                supportedPatterns.append( mimeType );
    }

    return supportedPatterns;
}

void Shell::fileOpen()
{
	// this slot is called whenever the File->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
    const int activeTab = m_tabWidget->currentIndex();
    if ( !m_fileformatsscanned )
    {
        const KDocumentViewer* const doc = qobject_cast<KDocumentViewer*>(m_tabs[activeTab].part);
        if ( doc )
            m_fileformats = doc->supportedMimeTypes();

        if ( m_fileformats.isEmpty() )
            m_fileformats = fileFormats();

        m_fileformatsscanned = true;
    }

    QString startDir;
    const KParts::ReadWritePart* const curPart = m_tabs[activeTab].part;
    if ( curPart->url().isLocalFile() )
        startDir = curPart->url().toLocalFile();
    KFileDialog dlg( startDir, QString(), this );
    dlg.setOperationMode( KFileDialog::Opening );

    // A directory may be a document. E.g. comicbook generator.
    if ( m_fileformats.contains( "inode/directory" ) )
        dlg.setMode( dlg.mode() | KFile::Directory );

    if ( m_fileformatsscanned && m_fileformats.isEmpty() )
        dlg.setFilter( i18n( "*|All Files" ) );
    else
        dlg.setMimeFilter( m_fileformats );
    dlg.setCaption( i18n( "Open Document" ) );
    if ( !dlg.exec() )
        return;
    KUrl url = dlg.selectedUrl();
    if ( !url.isEmpty() )
    {
        openUrl( url );
    }
}

void Shell::slotQuit()
{
    close();
}

void Shell::tryRaise()
{
    KWindowSystem::forceActiveWindow( window()->effectiveWinId() );
}

// only called when starting the program
void Shell::setFullScreen( bool useFullScreen )
{
    if( useFullScreen )
        setWindowState( windowState() | Qt::WindowFullScreen ); // set
    else
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset
}

void Shell::showEvent(QShowEvent *e)
{
    if (m_showMenuBarAction)
        m_showMenuBarAction->setChecked( menuBar()->isVisible() );

    KParts::MainWindow::showEvent(e);
}

void Shell::slotUpdateFullScreen()
{
    if(m_fullScreenAction->isChecked())
    {
      m_menuBarWasShown = !menuBar()->isHidden();
      menuBar()->hide();
      
      m_toolBarWasShown = !toolBar()->isHidden();
      toolBar()->hide();

      KToggleFullScreenAction::setFullScreen(this, true);      
    }
    else
    {
      if (m_menuBarWasShown)
      {
        menuBar()->show();
      }
      if (m_toolBarWasShown)
      {
        toolBar()->show();
      }
      KToggleFullScreenAction::setFullScreen(this, false);      
    }
}

void Shell::slotShowMenubar()
{
    if ( menuBar()->isHidden() )
        menuBar()->show();
    else
        menuBar()->hide();
}

QSize Shell::sizeHint() const
{
    return QApplication::desktop()->availableGeometry( this ).size() * 0.75;
}

bool Shell::queryClose()
{
    for( int i = 0; i < m_tabs.size(); ++i )
    {
        KParts::ReadWritePart* const part = m_tabs[i].part;

        // To resolve confusion about multiple modified docs, switch to relevant tab
        if( part->isModified() )
            setActiveTab( i );

        if( !part->queryClose() )
           return false;
    }
    return true;
}

void Shell::setActiveTab( int tab )
{
    m_tabWidget->setCurrentIndex( tab );
    createGUI( m_tabs[tab].part );
    m_printAction->setEnabled( m_tabs[tab].printEnabled );
    m_closeAction->setEnabled( m_tabs[tab].closeEnabled );
}

void Shell::closeTab( int tab )
{
    KParts::ReadWritePart* const part = m_tabs[tab].part;
    if( part->closeUrl() && m_tabs.count() > 1 )
    {
        if( part->factory() )
            part->factory()->removeClient( part );
        part->disconnect();
        part->deleteLater();
        m_tabs.removeAt( tab );
        m_tabWidget->removeTab( tab );

        if( m_tabWidget->count() == 1 )
        {
            m_tabWidget->setTabBarHidden( true );
            m_nextTabAction->setEnabled( false );
            m_prevTabAction->setEnabled( false );
        }
    }

}

void Shell::openNewTab( const KUrl& url, const QString &serializedOptions )
{
    // Tabs are hidden when there's only one, so show it
    if( m_tabs.size() == 1 )
    {
        m_tabWidget->setTabBarHidden( false );
        m_nextTabAction->setEnabled( true );
        m_prevTabAction->setEnabled( true );
    }

    const int newIndex = m_tabs.size();

    // Make new part
    m_tabs.append( m_partFactory->create<KParts::ReadWritePart>(this) );
    connectPart( m_tabs[newIndex].part );

    // Update GUI
    KParts::ReadWritePart* const part = m_tabs[newIndex].part;
    m_tabWidget->addTab( part->widget(), url.fileName() );

    applyOptionsToPart(part, serializedOptions);

    int previousActiveTab = m_tabWidget->currentIndex();
    setActiveTab( m_tabs.size() - 1 );

    if( part->openUrl(url) )
        m_recent->addUrl( url );
    else
        setActiveTab( previousActiveTab );
}

void Shell::applyOptionsToPart( QObject* part, const QString &serializedOptions )
{
    KDocumentViewer* const doc = qobject_cast<KDocumentViewer*>(part);
    if ( ShellUtils::startInPresentation(serializedOptions) )
        doc->startPresentation();
    if ( ShellUtils::showPrintDialog(serializedOptions) )
        QMetaObject::invokeMethod( part, "enableStartWithPrint" );
}

void Shell::connectPart( QObject* part )
{
    connect( this, SIGNAL(restoreDocument(KConfigGroup)), part, SLOT(restoreDocument(KConfigGroup)));
    connect( this, SIGNAL(saveDocumentRestoreInfo(KConfigGroup&)), part, SLOT(saveDocumentRestoreInfo(KConfigGroup&)));
    connect( part, SIGNAL(enablePrintAction(bool)), this, SLOT(setPrintEnabled(bool)));
    connect( part, SIGNAL(enableCloseAction(bool)), this, SLOT(setCloseEnabled(bool)));
    connect( part, SIGNAL(mimeTypeChanged(KMimeType::Ptr)), this, SLOT(setTabIcon(KMimeType::Ptr)));
    connect( part, SIGNAL(urlsDropped(KUrl::List)), this, SLOT(handleDroppedUrls(KUrl::List)) );
}

void Shell::print()
{
    QMetaObject::invokeMethod( m_tabs[m_tabWidget->currentIndex()].part, "slotPrint" );
}

void Shell::setPrintEnabled( bool enabled )
{
    int i = findTabIndex( sender() );
    if( i != -1 )
    {
        m_tabs[i].printEnabled = enabled;
        if( i == m_tabWidget->currentIndex() )
            m_printAction->setEnabled( enabled );
    }
}

void Shell::setCloseEnabled( bool enabled )
{
    int i = findTabIndex( sender() );
    if( i != -1 )
    {
        m_tabs[i].closeEnabled = enabled;
        if( i == m_tabWidget->currentIndex() )
            m_closeAction->setEnabled( enabled );
    }
}

void Shell::activateNextTab()
{
    if( m_tabs.size() < 2 )
        return;

    const int activeTab = m_tabWidget->currentIndex();
    const int nextTab = (activeTab == m_tabs.size()-1) ? 0 : activeTab+1;

    setActiveTab( nextTab );
}

void Shell::activatePrevTab()
{
    if( m_tabs.size() < 2 )
        return;

    const int activeTab = m_tabWidget->currentIndex();
    const int prevTab = (activeTab == 0) ? m_tabs.size()-1 : activeTab-1;

    setActiveTab( prevTab );
}

void Shell::setTabIcon( KMimeType::Ptr mimeType )
{
    int i = findTabIndex( sender() );
    if( i != -1 )
    {
        m_tabWidget->setTabIcon( i, KIcon(mimeType->iconName()) );
    }
}

int Shell::findTabIndex( QObject* sender )
{
    for( int i = 0; i < m_tabs.size(); ++i )
    {
        if( m_tabs[i].part == sender )
        {
            return i;
        }
    }
    return -1;
}

void Shell::handleDroppedUrls( const KUrl::List& urls )
{
    foreach( const KUrl& url, urls )
    {
        openUrl( url );
    }
}

void Shell::testTabDrop( const QDragMoveEvent* event, bool& accept )
{
    accept = KUrl::List::canDecode( event->mimeData() );
}

void Shell::handleTabDrop( QDropEvent* event )
{
    const KUrl::List list = KUrl::List::fromMimeData( event->mimeData() );
    handleDroppedUrls( list );
}

void Shell::moveTabData( int from, int to )
{
   m_tabs.move( from, to );
}

#include "shell.moc"

/* kate: replace-tabs on; indent-width 4; */
