/***************************************************************************
 *   Copyright (C) 2005-2014 by the Quassel Project                        *
 *   devel@quassel-irc.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3.                                           *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "mainwin.h"

#include <QIcon>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

#include <KAction>
#include <KActionCollection>
#include <KHelpMenu>
#include <KMenuBar>
#include <KShortcutsDialog>
#include <KStatusBar>
#include <KToggleFullScreenAction>
#include <KToolBar>
#include <KWindowSystem>
#include <KAboutApplicationDialog>
#include <KTipDialog>

#ifdef Q_WS_X11
#  include <QX11Info>
#endif

#include "awaylogfilter.h"
#include "awaylogview.h"
#include "action.h"
#include "actioncollection.h"
#include "bufferhotlistfilter.h"
#include "buffermodel.h"
#include "bufferview.h"
#include "bufferviewoverlay.h"
#include "bufferviewoverlayfilter.h"
#include "bufferwidget.h"
#include "channellistdlg.h"
#include "chatlinemodel.h"
#include "chatmonitorfilter.h"
#include "chatmonitorview.h"
#include "chatview.h"
#include "client.h"
#include "clientbacklogmanager.h"
#include "clientbufferviewconfig.h"
#include "clientbufferviewmanager.h"
#include "clientignorelistmanager.h"
#include "clienttransfer.h"
#include "clienttransfermanager.h"
#include "coreconfigwizard.h"
#include "coreconnectdlg.h"
#include "coreconnection.h"
#include "coreconnectionstatuswidget.h"
#include "coreinfodlg.h"
#include "contextmenuactionprovider.h"
#include "debugbufferviewoverlay.h"
#include "debugmessagemodelfilter.h"
#include "flatproxymodel.h"
#include "inputwidget.h"
#include "irclistmodel.h"
#include "ircconnectionwizard.h"
#include "legacysystemtray.h"
#include "msgprocessorstatuswidget.h"
#include "nicklistwidget.h"
#include "qtuiapplication.h"
#include "qtuimessageprocessor.h"
#include "qtuisettings.h"
#include "qtuistyle.h"
#include "receivefiledlg.h"
#include "settingsdlg.h"
#include "settingspagedlg.h"
#include "statusnotifieritem.h"
#include "toolbaractionprovider.h"
#include "topicwidget.h"
#include "verticaldock.h"
#include "knotificationbackend.h"

#ifdef HAVE_SSL
#  include "sslinfodlg.h"
#endif

#ifdef HAVE_INDICATEQT
  #include "indicatornotificationbackend.h"
#endif

#include "settingspages/aliasessettingspage.h"
#include "settingspages/appearancesettingspage.h"
#include "settingspages/backlogsettingspage.h"
#include "settingspages/bufferviewsettingspage.h"
#include "settingspages/chatmonitorsettingspage.h"
#include "settingspages/chatviewsettingspage.h"
#include "settingspages/connectionsettingspage.h"
#include "settingspages/coreaccountsettingspage.h"
#include "settingspages/coreconnectionsettingspage.h"
#include "settingspages/highlightsettingspage.h"
#include "settingspages/identitiessettingspage.h"
#include "settingspages/ignorelistsettingspage.h"
#include "settingspages/inputwidgetsettingspage.h"
#include "settingspages/itemviewsettingspage.h"
#include "settingspages/networkssettingspage.h"
#include "settingspages/notificationssettingspage.h"
#include "settingspages/topicwidgetsettingspage.h"


MainWin::MainWin(QWidget *parent)
    : KMainWindow(parent),
    _kHelpMenu(new KHelpMenu(this, KGlobal::mainComponent().aboutData())),
    _msgProcessorStatusWidget(new MsgProcessorStatusWidget(this)),
    _coreConnectionStatusWidget(new CoreConnectionStatusWidget(Client::coreConnection(), this)),
    _titleSetter(this),
    _awayLog(0),
    _layoutLoaded(false),
    _activeBufferViewIndex(-1)
{
    setAttribute(Qt::WA_DeleteOnClose, false); // we delete the mainwin manually

    QtUiSettings uiSettings;
    QString style = uiSettings.value("Style", QString()).toString();
    if (!style.isEmpty()) {
        QApplication::setStyle(style);
    }

    QApplication::setQuitOnLastWindowClosed(false);

    setWindowTitle("Kuassel IRC");
    setWindowIconText("Kuassel IRC");
    qApp->setWindowIcon(KIcon("kuassel"));
}


void MainWin::init()
{
    connect(Client::instance(), SIGNAL(networkCreated(NetworkId)), SLOT(clientNetworkCreated(NetworkId)));
    connect(Client::instance(), SIGNAL(networkRemoved(NetworkId)), SLOT(clientNetworkRemoved(NetworkId)));
    connect(Client::messageModel(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
        SLOT(messagesInserted(const QModelIndex &, int, int)));
    connect(GraphicalUi::contextMenuActionProvider(), SIGNAL(showChannelList(NetworkId)), SLOT(showChannelList(NetworkId)));
    connect(GraphicalUi::contextMenuActionProvider(), SIGNAL(showIgnoreList(QString)), SLOT(showIgnoreList(QString)));

    connect(Client::coreConnection(), SIGNAL(startCoreSetup(QVariantList)), SLOT(showCoreConfigWizard(QVariantList)));
    connect(Client::coreConnection(), SIGNAL(connectionErrorPopup(QString)), SLOT(handleCoreConnectionError(QString)));
    connect(Client::coreConnection(), SIGNAL(userAuthenticationRequired(CoreAccount *, bool *, QString)), SLOT(userAuthenticationRequired(CoreAccount *, bool *, QString)));
    connect(Client::coreConnection(), SIGNAL(handleNoSslInClient(bool *)), SLOT(handleNoSslInClient(bool *)));
    connect(Client::coreConnection(), SIGNAL(handleNoSslInCore(bool *)), SLOT(handleNoSslInCore(bool *)));
#ifdef HAVE_SSL
    connect(Client::coreConnection(), SIGNAL(handleSslErrors(const QSslSocket *, bool *, bool *)), SLOT(handleSslErrors(const QSslSocket *, bool *, bool *)));
#endif

    // Setup Dock Areas
    setDockNestingEnabled(true);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // Order is sometimes important
    setupActions();
    setupBufferWidget();
    setupMenus();
    setupTopicWidget();
    setupNickWidget();
    setupInputWidget();
    setupChatMonitor();
    setupViewMenuTail();
    setupStatusBar();
    setupToolBars();
    setupSystray();
    setupTitleSetter();
    setupHotList();

    QtUi::registerNotificationBackend(new KNotificationBackend(this));

#ifdef HAVE_INDICATEQT
    QtUi::registerNotificationBackend(new IndicatorNotificationBackend(this));
#endif

    // we assume that at this point, all configurable actions are defined!
    QtUi::loadShortcuts();

    connect(bufferWidget(), SIGNAL(currentChanged(BufferId)), SLOT(currentBufferChanged(BufferId)));

    setDisconnectedState(); // Disable menus and stuff

    setAutoSaveSettings();

    // restore mainwin state
    QtUiSettings s;
    restoreStateFromSettings(s);

    // restore locked state of docks
    QtUi::actionCollection("General")->action("LockLayout")->setChecked(s.value("LockLayout", false).toBool());

    CoreConnection *conn = Client::coreConnection();
    if (!conn->connectToCore()) {
        // No autoconnect selected (or no accounts)
        showCoreConnectionDlg();
    }

    // show the nice tips
    KTipDialog::showTip();

    if (Quassel::isOptionSet("url")) {
        // FIXME: connect to channel upon request from external KDE application
        kWarning(300000) << "non-interactive connection not yet implemented:" << Quassel::optionValue("url");
    }

}


MainWin::~MainWin()
{
}


void MainWin::quit()
{
    QtUiSettings s;
    saveStateToSettings(s);
    saveLayout();
    QApplication::quit();
}


void MainWin::saveStateToSettings(UiSettings &s)
{
    s.setValue("MainWinSize", _normalSize);
    s.setValue("MainWinPos", _normalPos);
    s.setValue("MainWinState", saveState());
    s.setValue("MainWinGeometry", saveGeometry());
    s.setValue("MainWinMinimized", isMinimized());
    s.setValue("MainWinMaximized", isMaximized());
    s.setValue("MainWinHidden", !isVisible());
    BufferId lastBufId = Client::bufferModel()->currentBuffer();
    if (lastBufId.isValid())
        s.setValue("LastUsedBufferId", lastBufId.toInt());

    saveAutoSaveSettings();
}


void MainWin::restoreStateFromSettings(UiSettings &s)
{
    _normalSize = s.value("MainWinSize", size()).toSize();
    _normalPos = s.value("MainWinPos", pos()).toPoint();
    bool maximized = s.value("MainWinMaximized", false).toBool();

    move(_normalPos);

    if ((Quassel::isOptionSet("hidewindow")
            || s.value("MainWinHidden").toBool())
            && _systemTray->isSystemTrayAvailable())
        QtUi::hideMainWidget();
    else if (s.value("MainWinMinimized").toBool())
        showMinimized();
    else if (maximized)
        showMaximized();
    else
        show();
}

void MainWin::setupActions()
{
    ActionCollection *coll = QtUi::actionCollection("General", i18n("General"));
    // File
    coll->addAction("ConnectCore", new Action(KIcon("network-connect"), i18n("&Connect to Core..."), coll,
            this, SLOT(showCoreConnectionDlg())));
    coll->addAction("DisconnectCore", new Action(KIcon("network-disconnect"), i18n("&Disconnect from Core"), coll,
            Client::instance(), SLOT(disconnectFromCore())));
    coll->addAction("CoreInfo", new Action(KIcon("help-about"), i18n("Core &Info..."), coll,
            this, SLOT(showCoreInfoDlg())));
    coll->addAction("ConfigureNetworks", new Action(KIcon("configure"), i18n("Configure &Networks..."), coll,
            this, SLOT(on_actionConfigureNetworks_triggered())));
    // FIXME: use QKeySequence::Quit once we depend on Qt 4.6
    coll->addAction("Quit", new Action(KIcon("application-exit"), i18n("&Quit"), coll,
            this, SLOT(quit()), Qt::CTRL + Qt::Key_Q));

    // View
    coll->addAction("ConfigureBufferViews", new Action(i18n("&Configure Chat Lists..."), coll,
            this, SLOT(on_actionConfigureViews_triggered())));

    QAction *lockAct = coll->addAction("LockLayout", new Action(i18n("&Lock Layout"), coll));
    lockAct->setCheckable(true);
    connect(lockAct, SIGNAL(toggled(bool)), SLOT(on_actionLockLayout_toggled(bool)));

    coll->addAction("ToggleSearchBar", new Action(KIcon("edit-find"), i18n("Show &Search Bar"), coll,
            0, 0, QKeySequence::Find))->setCheckable(true);
    coll->addAction("ShowAwayLog", new Action(i18n("Show Away Log"), coll,
            this, SLOT(showAwayLog())));
    coll->addAction("ToggleMenuBar", new Action(KIcon("show-menu"), i18n("Show &Menubar"), coll,
            0, 0, QKeySequence(Qt::CTRL + Qt::Key_M)))->setCheckable(true);

    coll->addAction("ToggleStatusBar", new Action(i18n("Show Status &Bar"), coll,
            0, 0))->setCheckable(true);

    QAction *fullScreenAct = KStandardAction::fullScreen(this, SLOT(onFullScreenToggled()), this, coll);
    coll->addAction("ToggleFullScreen", fullScreenAct);

    // Settings
    QAction *configureShortcutsAct = new Action(KIcon("configure-shortcuts"), i18n("Configure &Shortcuts..."), coll,
        this, SLOT(showShortcutsDlg()));
    configureShortcutsAct->setMenuRole(QAction::NoRole);
    coll->addAction("ConfigureShortcuts", configureShortcutsAct);

    QAction *configureQuasselAct = new Action(KIcon("configure"), i18n("&Configure Kuassel..."), coll,
        this, SLOT(showSettingsDlg()), QKeySequence(Qt::Key_F7));
    coll->addAction("ConfigureQuassel", configureQuasselAct);

    // Tools
    coll->addAction("DebugNetworkModel", new Action(KIcon("tools-report-bug"), i18n("Debug &NetworkModel"), coll,
            this, SLOT(on_actionDebugNetworkModel_triggered())));
    coll->addAction("DebugBufferViewOverlay", new Action(KIcon("tools-report-bug"), i18n("Debug &BufferViewOverlay"), coll,
            this, SLOT(on_actionDebugBufferViewOverlay_triggered())));
    coll->addAction("DebugMessageModel", new Action(KIcon("tools-report-bug"), i18n("Debug &MessageModel"), coll,
            this, SLOT(on_actionDebugMessageModel_triggered())));
    coll->addAction("DebugHotList", new Action(KIcon("tools-report-bug"), i18n("Debug &HotList"), coll,
            this, SLOT(on_actionDebugHotList_triggered())));
    coll->addAction("ReloadStyle", new Action(KIcon("view-refresh"), i18n("Reload Stylesheet"), coll,
            QtUi::style(), SLOT(reload()), QKeySequence::Refresh));
    coll->addAction("HideCurrentBuffer", new Action(i18n("Hide Current Buffer"), coll,
            this, SLOT(hideCurrentBuffer()), QKeySequence::Close));

    // Help
    QAction *aboutQuasselAct = new Action(KIcon("kuassel"), i18n("&About Kuassel"), coll,
        this, SLOT(showAboutDlg()));
    aboutQuasselAct->setMenuRole(QAction::AboutRole);
    coll->addAction("AboutQuassel", aboutQuasselAct);

    // Navigation
    coll = QtUi::actionCollection("Navigation", i18n("Navigation"));

    coll->addAction("JumpHotBuffer", new Action(i18n("Jump to hot chat"), coll,
            this, SLOT(on_jumpHotBuffer_triggered()), QKeySequence(Qt::META + Qt::Key_A)));

    // Jump keys
    const int bindModifier = Qt::ControlModifier;
    const int jumpModifier = Qt::AltModifier;

    coll->addAction("BindJumpKey0", new Action(i18n("Set Quick Access #0"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_0)))->setProperty("Index", 0);
    coll->addAction("BindJumpKey1", new Action(i18n("Set Quick Access #1"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_1)))->setProperty("Index", 1);
    coll->addAction("BindJumpKey2", new Action(i18n("Set Quick Access #2"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_2)))->setProperty("Index", 2);
    coll->addAction("BindJumpKey3", new Action(i18n("Set Quick Access #3"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_3)))->setProperty("Index", 3);
    coll->addAction("BindJumpKey4", new Action(i18n("Set Quick Access #4"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_4)))->setProperty("Index", 4);
    coll->addAction("BindJumpKey5", new Action(i18n("Set Quick Access #5"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_5)))->setProperty("Index", 5);
    coll->addAction("BindJumpKey6", new Action(i18n("Set Quick Access #6"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_6)))->setProperty("Index", 6);
    coll->addAction("BindJumpKey7", new Action(i18n("Set Quick Access #7"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_7)))->setProperty("Index", 7);
    coll->addAction("BindJumpKey8", new Action(i18n("Set Quick Access #8"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_8)))->setProperty("Index", 8);
    coll->addAction("BindJumpKey9", new Action(i18n("Set Quick Access #9"), coll, this, SLOT(bindJumpKey()),
            QKeySequence(bindModifier + Qt::Key_9)))->setProperty("Index", 9);

    coll->addAction("JumpKey0", new Action(i18n("Quick Access #0"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_0)))->setProperty("Index", 0);
    coll->addAction("JumpKey1", new Action(i18n("Quick Access #1"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_1)))->setProperty("Index", 1);
    coll->addAction("JumpKey2", new Action(i18n("Quick Access #2"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_2)))->setProperty("Index", 2);
    coll->addAction("JumpKey3", new Action(i18n("Quick Access #3"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_3)))->setProperty("Index", 3);
    coll->addAction("JumpKey4", new Action(i18n("Quick Access #4"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_4)))->setProperty("Index", 4);
    coll->addAction("JumpKey5", new Action(i18n("Quick Access #5"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_5)))->setProperty("Index", 5);
    coll->addAction("JumpKey6", new Action(i18n("Quick Access #6"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_6)))->setProperty("Index", 6);
    coll->addAction("JumpKey7", new Action(i18n("Quick Access #7"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_7)))->setProperty("Index", 7);
    coll->addAction("JumpKey8", new Action(i18n("Quick Access #8"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_8)))->setProperty("Index", 8);
    coll->addAction("JumpKey9", new Action(i18n("Quick Access #9"), coll, this, SLOT(onJumpKey()),
            QKeySequence(jumpModifier + Qt::Key_9)))->setProperty("Index", 9);

    // Buffer navigation
    coll->addAction("NextBufferView", new Action(KIcon("go-next-view"), i18n("Activate Next Chat List"), coll,
            this, SLOT(nextBufferView()), QKeySequence(QKeySequence::Forward)));
    coll->addAction("PreviousBufferView", new Action(KIcon("go-previous-view"), i18n("Activate Previous Chat List"), coll,
            this, SLOT(previousBufferView()), QKeySequence::Back));
    coll->addAction("NextBuffer", new Action(KIcon("go-down"), i18n("Go to Next Chat"), coll,
            this, SLOT(nextBuffer()), QKeySequence(Qt::ALT + Qt::Key_Down)));
    coll->addAction("PreviousBuffer", new Action(KIcon("go-up"), i18n("Go to Previous Chat"), coll,
            this, SLOT(previousBuffer()), QKeySequence(Qt::ALT + Qt::Key_Up)));
}


void MainWin::setupMenus()
{
    ActionCollection *coll = QtUi::actionCollection("General");

    _fileMenu = menuBar()->addMenu(i18n("&File"));

    static const QStringList coreActions = QStringList()
                                           << "ConnectCore" << "DisconnectCore" << "CoreInfo";

    QAction *coreAction;
    foreach(QString actionName, coreActions) {
        coreAction = coll->action(actionName);
        _fileMenu->addAction(coreAction);
        flagRemoteCoreOnly(coreAction);
    }
    flagRemoteCoreOnly(_fileMenu->addSeparator());

    _networksMenu = _fileMenu->addMenu(i18n("&Networks"));
    _networksMenu->addAction(coll->action("ConfigureNetworks"));
    _networksMenu->addSeparator();
    _fileMenu->addSeparator();
    _fileMenu->addAction(coll->action("Quit"));

    _viewMenu = menuBar()->addMenu(i18n("&View"));
    _bufferViewsMenu = _viewMenu->addMenu(i18n("&Chat Lists"));
    _bufferViewsMenu->addAction(coll->action("ConfigureBufferViews"));
    _toolbarMenu = _viewMenu->addMenu(i18n("&Toolbars"));
    _viewMenu->addSeparator();

    _viewMenu->addAction(coll->action("ToggleMenuBar"));
    _viewMenu->addAction(coll->action("ToggleStatusBar"));
    _viewMenu->addAction(coll->action("ToggleSearchBar"));

    coreAction = coll->action("ShowAwayLog");
    flagRemoteCoreOnly(coreAction);
    _viewMenu->addAction(coreAction);

    _viewMenu->addSeparator();
    _viewMenu->addAction(coll->action("LockLayout"));

    _settingsMenu = menuBar()->addMenu(i18n("&Settings"));
    _settingsMenu->addAction(KStandardAction::configureNotifications(this, SLOT(showNotificationsDlg()), this));
    _settingsMenu->addAction(KStandardAction::keyBindings(this, SLOT(showShortcutsDlg()), this));
    _settingsMenu->addAction(coll->action("ConfigureQuassel"));

    _helpDebugMenu = menuBar()->addMenu(i18n("&Tools"));
    _helpDebugMenu->addAction(coll->action("DebugNetworkModel"));
    _helpDebugMenu->addAction(coll->action("DebugBufferViewOverlay"));
    _helpDebugMenu->addAction(coll->action("DebugMessageModel"));
    _helpDebugMenu->addAction(coll->action("DebugHotList"));
    _helpDebugMenu->addSeparator();
    _helpDebugMenu->addAction(coll->action("ReloadStyle"));

    _helpMenu = menuBar()->addMenu(i18n("&Help"));
    _helpMenu->addAction(KStandardAction::whatsThis(_kHelpMenu, SLOT(contextHelpActivated()), this));
    _helpMenu->addSeparator();
#warning "implement tips-of-day action from help menu"
    // _helpMenu->addAction(KStandardAction::tipOfDay(_kHelpMenu, SLOT(tipOfDay()), this));
    // _helpMenu->addSeparator();
    _helpMenu->addAction(KStandardAction::reportBug(_kHelpMenu, SLOT(reportBug()), this));
    _helpMenu->addSeparator();
    _helpMenu->addAction(KStandardAction::switchApplicationLanguage(_kHelpMenu, SLOT(switchApplicationLanguage()), this));
    _helpMenu->addSeparator();
    _helpMenu->addAction(coll->action("AboutQuassel"));
    _helpMenu->addAction(KStandardAction::aboutKDE(_kHelpMenu, SLOT(aboutKDE()), this));
    _helpMenu->addSeparator();

    // Toggle visibility
    QAction *showMenuBar = QtUi::actionCollection("General")->action("ToggleMenuBar");

    QtUiSettings uiSettings;
    bool enabled = uiSettings.value("ShowMenuBar", QVariant(true)).toBool();
    showMenuBar->setChecked(enabled);
    enabled ? menuBar()->show() : menuBar()->hide();

    connect(showMenuBar, SIGNAL(toggled(bool)), menuBar(), SLOT(setVisible(bool)));
    connect(showMenuBar, SIGNAL(toggled(bool)), this, SLOT(saveMenuBarStatus(bool)));
}


void MainWin::setupBufferWidget()
{
    _bufferWidget = new BufferWidget(this);
    _bufferWidget->setModel(Client::bufferModel());
    _bufferWidget->setSelectionModel(Client::bufferModel()->standardSelectionModel());
    setCentralWidget(_bufferWidget);
}


void MainWin::addBufferView(int bufferViewConfigId)
{
    addBufferView(Client::bufferViewManager()->clientBufferViewConfig(bufferViewConfigId));
}


void MainWin::addBufferView(ClientBufferViewConfig *config)
{
    if (!config)
        return;

    config->setLocked(QtUiSettings().value("LockLayout", false).toBool());
    BufferViewDock *dock = new BufferViewDock(config, this);

    //create the view and initialize it's filter
    BufferView *view = new BufferView(dock);
    view->setFilteredModel(Client::bufferModel(), config);
    view->installEventFilter(_inputWidget); // for key presses

    Client::bufferModel()->synchronizeView(view);

    dock->setWidget(view);
    dock->setVisible(_layoutLoaded); // don't show before state has been restored

    addDockWidget(Qt::LeftDockWidgetArea, dock);
    _bufferViewsMenu->addAction(dock->toggleViewAction());

    connect(dock->toggleViewAction(), SIGNAL(toggled(bool)), this, SLOT(bufferViewToggled(bool)));
    connect(dock, SIGNAL(visibilityChanged(bool)), SLOT(bufferViewVisibilityChanged(bool)));
    _bufferViews.append(dock);

    if (!activeBufferView())
        nextBufferView();
}


void MainWin::removeBufferView(int bufferViewConfigId)
{
    QVariant actionData;
    BufferViewDock *dock;
    foreach(QAction *action, _bufferViewsMenu->actions()) {
        actionData = action->data();
        if (!actionData.isValid())
            continue;

        dock = qobject_cast<BufferViewDock *>(action->parent());
        if (dock && actionData.toInt() == bufferViewConfigId) {
            removeAction(action);
            Client::bufferViewOverlay()->removeView(dock->bufferViewId());
            _bufferViews.removeAll(dock);

            if (dock->isActive()) {
                dock->setActive(false);
                _activeBufferViewIndex = -1;
                nextBufferView();
            }

            dock->deleteLater();
        }
    }
}


void MainWin::bufferViewToggled(bool enabled)
{
    if (!enabled && !isMinimized()) {
        // hiding the mainwindow triggers a toggle of the bufferview (which pretty much sucks big time)
        // since this isn't our fault and we can't do anything about it, we suppress the resulting calls
        return;
    }
    QAction *action = qobject_cast<QAction *>(sender());
    Q_ASSERT(action);
    BufferViewDock *dock = qobject_cast<BufferViewDock *>(action->parent());
    Q_ASSERT(dock);

    // Make sure we don't toggle backlog fetch for a view we've already removed
    if (!_bufferViews.contains(dock))
        return;

    if (enabled)
        Client::bufferViewOverlay()->addView(dock->bufferViewId());
    else
        Client::bufferViewOverlay()->removeView(dock->bufferViewId());
}


void MainWin::bufferViewVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);
    BufferViewDock *dock = qobject_cast<BufferViewDock *>(sender());
    Q_ASSERT(dock);
    if ((!dock->isHidden() && !activeBufferView()) || (dock->isHidden() && dock->isActive()))
        nextBufferView();
}


BufferView *MainWin::allBuffersView() const
{
    // "All Buffers" is always the first dock created
    if (_bufferViews.count() > 0)
        return _bufferViews[0]->bufferView();
    return 0;
}


BufferView *MainWin::activeBufferView() const
{
    if (_activeBufferViewIndex < 0 || _activeBufferViewIndex >= _bufferViews.count())
        return 0;
    BufferViewDock *dock = _bufferViews.at(_activeBufferViewIndex);
    return dock->isActive() ? qobject_cast<BufferView *>(dock->widget()) : 0;
}


void MainWin::changeActiveBufferView(int bufferViewId)
{
    if (bufferViewId < 0)
        return;

    BufferView *current = activeBufferView();
    if (current) {
        qobject_cast<BufferViewDock *>(current->parent())->setActive(false);
        _activeBufferViewIndex = -1;
    }

    for (int i = 0; i < _bufferViews.count(); i++) {
        BufferViewDock *dock = _bufferViews.at(i);
        if (dock->bufferViewId() == bufferViewId && !dock->isHidden()) {
            _activeBufferViewIndex = i;
            dock->setActive(true);
            return;
        }
    }

    nextBufferView(); // fallback
}


void MainWin::changeActiveBufferView(bool backwards)
{
    BufferView *current = activeBufferView();
    if (current)
        qobject_cast<BufferViewDock *>(current->parent())->setActive(false);

    if (!_bufferViews.count())
        return;

    int c = _bufferViews.count();
    while (c--) { // yes, this will reactivate the current active one if all others fail
        if (backwards) {
            if (--_activeBufferViewIndex < 0)
                _activeBufferViewIndex = _bufferViews.count()-1;
        }
        else {
            if (++_activeBufferViewIndex >= _bufferViews.count())
                _activeBufferViewIndex = 0;
        }

        BufferViewDock *dock = _bufferViews.at(_activeBufferViewIndex);
        if (dock->isHidden())
            continue;

        dock->setActive(true);
        return;
    }

    _activeBufferViewIndex = -1;
}


void MainWin::nextBufferView()
{
    changeActiveBufferView(false);
}


void MainWin::previousBufferView()
{
    changeActiveBufferView(true);
}


void MainWin::nextBuffer()
{
    BufferView *view = activeBufferView();
    if (view)
        view->nextBuffer();
}


void MainWin::previousBuffer()
{
    BufferView *view = activeBufferView();
    if (view)
        view->previousBuffer();
}


void MainWin::hideCurrentBuffer()
{
    BufferView *view = activeBufferView();
    if (view)
        view->hideCurrentBuffer();
}


void MainWin::showNotificationsDlg()
{
    SettingsPageDlg dlg(new NotificationsSettingsPage(this), this);
    dlg.exec();
}


void MainWin::on_actionConfigureNetworks_triggered()
{
    SettingsPageDlg dlg(new NetworksSettingsPage(this), this);
    dlg.exec();
}


void MainWin::on_actionConfigureViews_triggered()
{
    SettingsPageDlg dlg(new BufferViewSettingsPage(this), this);
    dlg.exec();
}


void MainWin::on_actionLockLayout_toggled(bool lock)
{
    QList<VerticalDock *> docks = findChildren<VerticalDock *>();
    foreach(VerticalDock *dock, docks) {
        dock->showTitle(!lock);
    }
    if (Client::bufferViewManager()) {
        foreach(ClientBufferViewConfig *config, Client::bufferViewManager()->clientBufferViewConfigs()) {
            config->setLocked(lock);
        }
    }
    QtUiSettings().setValue("LockLayout", lock);
}


void MainWin::setupNickWidget()
{
    // create nick dock
    NickListDock *nickDock = new NickListDock(i18n("Nicks"), this);
    nickDock->setObjectName("NickDock");
    nickDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    _nickListWidget = new NickListWidget(nickDock);
    nickDock->setWidget(_nickListWidget);

    addDockWidget(Qt::RightDockWidgetArea, nickDock);
    _viewMenu->addAction(nickDock->toggleViewAction());
    nickDock->toggleViewAction()->setText(i18n("Show Nick List"));

    // See NickListDock::NickListDock();
    // connect(nickDock->toggleViewAction(), SIGNAL(triggered(bool)), nickListWidget, SLOT(showWidget(bool)));

    // attach the NickListWidget to the BufferModel and the default selection
    _nickListWidget->setModel(Client::bufferModel());
    _nickListWidget->setSelectionModel(Client::bufferModel()->standardSelectionModel());
}


void MainWin::setupChatMonitor()
{
    VerticalDock *dock = new VerticalDock(i18n("Chat Monitor"), this);
    dock->setObjectName("ChatMonitorDock");

    ChatMonitorFilter *filter = new ChatMonitorFilter(Client::messageModel(), this);
    _chatMonitorView = new ChatMonitorView(filter, this);
    _chatMonitorView->setFocusProxy(_inputWidget);
    _chatMonitorView->show();
    dock->setWidget(_chatMonitorView);
    dock->hide();

    addDockWidget(Qt::TopDockWidgetArea, dock, Qt::Vertical);
    _viewMenu->addAction(dock->toggleViewAction());
    dock->toggleViewAction()->setText(i18n("Show Chat Monitor"));
}


void MainWin::setupInputWidget()
{
    VerticalDock *dock = new VerticalDock(i18n("Inputline"), this);
    dock->setObjectName("InputDock");

    _inputWidget = new InputWidget(dock);
    dock->setWidget(_inputWidget);

    addDockWidget(Qt::BottomDockWidgetArea, dock);

    _viewMenu->addAction(dock->toggleViewAction());
    dock->toggleViewAction()->setText(i18n("Show Input Line"));

    _inputWidget->setModel(Client::bufferModel());
    _inputWidget->setSelectionModel(Client::bufferModel()->standardSelectionModel());

    _bufferWidget->setFocusProxy(_inputWidget);

    _inputWidget->inputLine()->installEventFilter(_bufferWidget);

    connect(_topicWidget, SIGNAL(switchedPlain()), _bufferWidget, SLOT(setFocus()));
}


void MainWin::setupTopicWidget()
{
    VerticalDock *dock = new VerticalDock(i18n("Topic"), this);
    dock->setObjectName("TopicDock");
    _topicWidget = new TopicWidget(dock);

    dock->setWidget(_topicWidget);

    _topicWidget->setModel(Client::bufferModel());
    _topicWidget->setSelectionModel(Client::bufferModel()->standardSelectionModel());

    addDockWidget(Qt::TopDockWidgetArea, dock, Qt::Vertical);

    _viewMenu->addAction(dock->toggleViewAction());
    dock->toggleViewAction()->setText(i18n("Show Topic Line"));
}


void MainWin::setupViewMenuTail()
{
    _viewMenu->addSeparator();
    _viewMenu->addAction(QtUi::actionCollection("General")->action("ToggleFullScreen"));
}


void MainWin::setupTitleSetter()
{
    _titleSetter.setModel(Client::bufferModel());
    _titleSetter.setSelectionModel(Client::bufferModel()->standardSelectionModel());
}


void MainWin::setupStatusBar()
{
    // MessageProcessor progress
    statusBar()->addPermanentWidget(_msgProcessorStatusWidget);

    // Connection state
    _coreConnectionStatusWidget->update();
    statusBar()->addPermanentWidget(_coreConnectionStatusWidget);

    QAction *showStatusbar = QtUi::actionCollection("General")->action("ToggleStatusBar");

    QtUiSettings uiSettings;

    bool enabled = uiSettings.value("ShowStatusBar", QVariant(true)).toBool();
    showStatusbar->setChecked(enabled);
    enabled ? statusBar()->show() : statusBar()->hide();

    connect(showStatusbar, SIGNAL(toggled(bool)), statusBar(), SLOT(setVisible(bool)));
    connect(showStatusbar, SIGNAL(toggled(bool)), this, SLOT(saveStatusBarStatus(bool)));

    connect(Client::coreConnection(), SIGNAL(connectionMsg(QString)), statusBar(), SLOT(showMessage(QString)));
}


void MainWin::setupHotList()
{
    FlatProxyModel *flatProxy = new FlatProxyModel(this);
    flatProxy->setSourceModel(Client::bufferModel());
    _bufferHotList = new BufferHotListFilter(flatProxy);
}


void MainWin::saveMenuBarStatus(bool enabled)
{
    QtUiSettings uiSettings;
    uiSettings.setValue("ShowMenuBar", enabled);
}


void MainWin::saveStatusBarStatus(bool enabled)
{
    QtUiSettings uiSettings;
    uiSettings.setValue("ShowStatusBar", enabled);
}


void MainWin::setupSystray()
{
#ifdef HAVE_DBUS
    _systemTray = new StatusNotifierItem(this);
#elif !defined QT_NO_SYSTEMTRAYICON
    _systemTray = new LegacySystemTray(this);
#else
    _systemTray = new SystemTray(this); // dummy
#endif
    _systemTray->init();
}


void MainWin::setupToolBars()
{
    connect(_bufferWidget, SIGNAL(currentChanged(QModelIndex)),
        QtUi::toolBarActionProvider(), SLOT(currentBufferChanged(QModelIndex)));
    connect(_nickListWidget, SIGNAL(nickSelectionChanged(QModelIndexList)),
        QtUi::toolBarActionProvider(), SLOT(nickSelectionChanged(QModelIndexList)));


    _mainToolBar = new KToolBar("MainToolBar", this, Qt::TopToolBarArea, false, true, true);
    _mainToolBar->setWindowTitle(i18n("Main Toolbar"));
    addToolBar(_mainToolBar);

    QtUi::toolBarActionProvider()->addActions(_mainToolBar, ToolBarActionProvider::MainToolBar);
    _toolbarMenu->addAction(_mainToolBar->toggleViewAction());

}

void MainWin::saveMainToolBarStatus(bool enabled)
{
    Q_UNUSED(enabled);
}


void MainWin::connectedToCore()
{
    Q_CHECK_PTR(Client::bufferViewManager());
    connect(Client::bufferViewManager(), SIGNAL(bufferViewConfigAdded(int)), this, SLOT(addBufferView(int)));
    connect(Client::bufferViewManager(), SIGNAL(bufferViewConfigDeleted(int)), this, SLOT(removeBufferView(int)));
    connect(Client::bufferViewManager(), SIGNAL(initDone()), this, SLOT(loadLayout()));

    connect(Client::transferManager(), SIGNAL(transferAdded(const ClientTransfer*)), SLOT(showNewTransferDlg(const ClientTransfer*)));

    setConnectedState();
}


void MainWin::setConnectedState()
{
    ActionCollection *coll = QtUi::actionCollection("General");

    coll->action("ConnectCore")->setEnabled(false);
    coll->action("DisconnectCore")->setEnabled(true);
    coll->action("CoreInfo")->setEnabled(true);

    foreach(QAction *action, _fileMenu->actions()) {
        if (isRemoteCoreOnly(action))
            action->setVisible(!Client::internalCore());
    }

    disconnect(Client::backlogManager(), SIGNAL(updateProgress(int, int)), _msgProcessorStatusWidget, SLOT(setProgress(int, int)));
    disconnect(Client::backlogManager(), SIGNAL(messagesRequested(const QString &)), this, SLOT(showStatusBarMessage(const QString &)));
    disconnect(Client::backlogManager(), SIGNAL(messagesProcessed(const QString &)), this, SLOT(showStatusBarMessage(const QString &)));
    if (!Client::internalCore()) {
        connect(Client::backlogManager(), SIGNAL(updateProgress(int, int)), _msgProcessorStatusWidget, SLOT(setProgress(int, int)));
        connect(Client::backlogManager(), SIGNAL(messagesRequested(const QString &)), this, SLOT(showStatusBarMessage(const QString &)));
        connect(Client::backlogManager(), SIGNAL(messagesProcessed(const QString &)), this, SLOT(showStatusBarMessage(const QString &)));
    }

    // _viewMenu->setEnabled(true);
    if (!Client::internalCore())
        statusBar()->showMessage(i18n("Connected to core."));
    else
        statusBar()->clearMessage();

    _coreConnectionStatusWidget->setVisible(!Client::internalCore());
    systemTray()->setState(SystemTray::Active);

    if (Client::networkIds().isEmpty()) {
        IrcConnectionWizard *wizard = new IrcConnectionWizard(this, Qt::Sheet);
        wizard->show();
    }
    else {
        // Monolithic always preselects last used buffer - Client only if the connection died
        if (Client::coreConnection()->wasReconnect() || Quassel::runMode() == Quassel::Monolithic) {
            QtUiSettings s;
            BufferId lastUsedBufferId(s.value("LastUsedBufferId").toInt());
            if (lastUsedBufferId.isValid())
                Client::bufferModel()->switchToBuffer(lastUsedBufferId);
        }
    }
}


void MainWin::loadLayout()
{
    QtUiSettings s;
    int accountId = Client::currentCoreAccount().accountId().toInt();
    QByteArray state = s.value(QString("MainWinState-%1").arg(accountId)).toByteArray();
    if (state.isEmpty()) {
        foreach(BufferViewDock *view, _bufferViews)
        view->show();
        _layoutLoaded = true;
        return;
    }

    restoreState(state, accountId);
    int bufferViewId = s.value(QString("ActiveBufferView-%1").arg(accountId), -1).toInt();
    if (bufferViewId >= 0)
        changeActiveBufferView(bufferViewId);

    _layoutLoaded = true;
}


void MainWin::saveLayout()
{
    QtUiSettings s;
    int accountId = _bufferViews.count() ? Client::currentCoreAccount().accountId().toInt() : 0; // only save if we still have a layout!
    if (accountId > 0) {
        s.setValue(QString("MainWinState-%1").arg(accountId), saveState(accountId));
        BufferView *view = activeBufferView();
        s.setValue(QString("ActiveBufferView-%1").arg(accountId), view ? view->config()->bufferViewId() : -1);
    }
}


void MainWin::disconnectedFromCore()
{
    // save core specific layout and remove bufferviews;
    saveLayout();
    _layoutLoaded = false;

    QVariant actionData;
    BufferViewDock *dock;
    foreach(QAction *action, _bufferViewsMenu->actions()) {
        actionData = action->data();
        if (!actionData.isValid())
            continue;

        dock = qobject_cast<BufferViewDock *>(action->parent());
        if (dock && actionData.toInt() != -1) {
            removeAction(action);
            _bufferViews.removeAll(dock);
            dock->deleteLater();
        }
    }

    // store last active buffer
    QtUiSettings s;
    BufferId lastBufId = _bufferWidget->currentBuffer();
    if (lastBufId.isValid()) {
        s.setValue("LastUsedBufferId", lastBufId.toInt());
        // clear the current selection
        Client::bufferModel()->standardSelectionModel()->clearSelection();
    }
    restoreState(s.value("MainWinState").toByteArray());
    setDisconnectedState();
}


void MainWin::setDisconnectedState()
{
    ActionCollection *coll = QtUi::actionCollection("General");
    //ui.menuCore->setEnabled(false);
    coll->action("ConnectCore")->setEnabled(true);
    coll->action("DisconnectCore")->setEnabled(false);
    coll->action("CoreInfo")->setEnabled(false);
    //_viewMenu->setEnabled(false);
    statusBar()->showMessage(i18n("Not connected to core."));
    if (_msgProcessorStatusWidget)
        _msgProcessorStatusWidget->setProgress(0, 0);
    systemTray()->setState(SystemTray::Passive);
}


void MainWin::userAuthenticationRequired(CoreAccount *account, bool *valid, const QString &errorMessage)
{
    Q_UNUSED(errorMessage)
    CoreConnectAuthDlg dlg(account, this);
    *valid = (dlg.exec() == QDialog::Accepted);
}


void MainWin::handleNoSslInClient(bool *accepted)
{
    QMessageBox box(QMessageBox::Warning, i18n("Unencrypted Connection"), i18n("<b>Your client does not support SSL encryption</b>"),
        QMessageBox::Ignore|QMessageBox::Cancel, this);
    box.setInformativeText(i18n("Sensitive data, like passwords, will be transmitted unencrypted to your Quassel core."));
    box.setDefaultButton(QMessageBox::Ignore);
    *accepted = box.exec() == QMessageBox::Ignore;
}


void MainWin::handleNoSslInCore(bool *accepted)
{
    QMessageBox box(QMessageBox::Warning, i18n("Unencrypted Connection"), i18n("<b>Your core does not support SSL encryption</b>"),
        QMessageBox::Ignore|QMessageBox::Cancel, this);
    box.setInformativeText(i18n("Sensitive data, like passwords, will be transmitted unencrypted to your Quassel core."));
    box.setDefaultButton(QMessageBox::Ignore);
    *accepted = box.exec() == QMessageBox::Ignore;
}


#ifdef HAVE_SSL

void MainWin::handleSslErrors(const QSslSocket *socket, bool *accepted, bool *permanently)
{
    QString errorString = "<ul>";
    foreach(const QSslError error, socket->sslErrors())
    errorString += QString("<li>%1</li>").arg(error.errorString());
    errorString += "</ul>";

    QMessageBox box(QMessageBox::Warning,
        i18n("Untrusted Security Certificate"),
        i18n("<b>The SSL certificate provided by the core at %1 is untrusted for the following reasons:</b>").arg(socket->peerName()),
        QMessageBox::Cancel, this);
    box.setInformativeText(errorString);
    box.addButton(i18n("Continue"), QMessageBox::AcceptRole);
    box.setDefaultButton(box.addButton(i18n("Show Certificate"), QMessageBox::HelpRole));

    QMessageBox::ButtonRole role;
    do {
        box.exec();
        role = box.buttonRole(box.clickedButton());
        if (role == QMessageBox::HelpRole) {
            SslInfoDlg dlg(socket, this);
            dlg.exec();
        }
    }
    while (role == QMessageBox::HelpRole);

    *accepted = role == QMessageBox::AcceptRole;
    if (*accepted) {
        QMessageBox box2(QMessageBox::Warning,
            i18n("Untrusted Security Certificate"),
            i18n("Would you like to accept this certificate forever without being prompted?"),
            0, this);
        box2.setDefaultButton(box2.addButton(i18n("Current Session Only"), QMessageBox::NoRole));
        box2.addButton(i18n("Forever"), QMessageBox::YesRole);
        box2.exec();
        *permanently =  box2.buttonRole(box2.clickedButton()) == QMessageBox::YesRole;
    }
}


#endif /* HAVE_SSL */

void MainWin::handleCoreConnectionError(const QString &error)
{
    QMessageBox::critical(this, i18n("Core Connection Error"), error, QMessageBox::Ok);
}


void MainWin::showCoreConnectionDlg()
{
    CoreConnectDlg dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        AccountId accId = dlg.selectedAccount();
        if (accId.isValid())
            Client::coreConnection()->connectToCore(accId);
    }
}


void MainWin::showCoreConfigWizard(const QVariantList &backends)
{
    CoreConfigWizard *wizard = new CoreConfigWizard(Client::coreConnection(), backends, this);

    wizard->show();
}


void MainWin::showChannelList(NetworkId netId)
{
    ChannelListDlg *channelListDlg = new ChannelListDlg();

    if (!netId.isValid()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            netId = action->data().value<NetworkId>();
    }

    channelListDlg->setAttribute(Qt::WA_DeleteOnClose);
    channelListDlg->setNetwork(netId);
    channelListDlg->show();
}


void MainWin::showIgnoreList(QString newRule)
{
    SettingsPageDlg dlg(new IgnoreListSettingsPage(this), this);
    // prepare config dialog for new rule
    if (!newRule.isEmpty())
        qobject_cast<IgnoreListSettingsPage *>(dlg.currentPage())->editIgnoreRule(newRule);
    dlg.exec();
}


void MainWin::showCoreInfoDlg()
{
    CoreInfoDlg(this).exec();
}


void MainWin::showAwayLog()
{
    if (_awayLog)
        return;
    AwayLogFilter *filter = new AwayLogFilter(Client::messageModel());
    _awayLog = new AwayLogView(filter, 0);
    filter->setParent(_awayLog);
    connect(_awayLog, SIGNAL(destroyed()), this, SLOT(awayLogDestroyed()));
    _awayLog->setAttribute(Qt::WA_DeleteOnClose);
    _awayLog->show();
}


void MainWin::awayLogDestroyed()
{
    _awayLog = 0;
}


void MainWin::showSettingsDlg()
{
    SettingsDlg *dlg = new SettingsDlg();

    //Category: Interface
    dlg->registerSettingsPage(new AppearanceSettingsPage(dlg));
    dlg->registerSettingsPage(new ChatViewSettingsPage(dlg));
    dlg->registerSettingsPage(new ChatMonitorSettingsPage(dlg));
    dlg->registerSettingsPage(new ItemViewSettingsPage(dlg));
    dlg->registerSettingsPage(new BufferViewSettingsPage(dlg));
    dlg->registerSettingsPage(new InputWidgetSettingsPage(dlg));
    dlg->registerSettingsPage(new TopicWidgetSettingsPage(dlg));
    dlg->registerSettingsPage(new HighlightSettingsPage(dlg));
    dlg->registerSettingsPage(new NotificationsSettingsPage(dlg));
    dlg->registerSettingsPage(new BacklogSettingsPage(dlg));

    //Category: IRC
    dlg->registerSettingsPage(new ConnectionSettingsPage(dlg));
    dlg->registerSettingsPage(new IdentitiesSettingsPage(dlg));
    dlg->registerSettingsPage(new NetworksSettingsPage(dlg));
    dlg->registerSettingsPage(new AliasesSettingsPage(dlg));
    dlg->registerSettingsPage(new IgnoreListSettingsPage(dlg));

    // Category: Remote Cores
    if (Quassel::runMode() != Quassel::Monolithic) {
        dlg->registerSettingsPage(new CoreAccountSettingsPage(dlg));
        dlg->registerSettingsPage(new CoreConnectionSettingsPage(dlg));
    }

    dlg->show();
}


void MainWin::showAboutDlg()
{
    KAboutApplicationDialog ad(KGlobal::mainComponent().aboutData(),this);
    ad.exec();
}


void MainWin::showShortcutsDlg()
{
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed, this);
    foreach(KActionCollection *coll, QtUi::actionCollections())
    dlg.addCollection(coll, coll->property("Category").toString());
    dlg.exec();
}


void MainWin::showNewTransferDlg(const ClientTransfer *transfer)
{
    ReceiveFileDlg *dlg = new ReceiveFileDlg(transfer, this);
    dlg->show();
}


void MainWin::onFullScreenToggled()
{
    // Relying on QWidget::isFullScreen is discouraged, see the KToggleFullScreenAction docs
    // Also, one should not use showFullScreen() or showNormal(), as those reset all other window flags

    QAction *action = QtUi::actionCollection("General")->action("ToggleFullScreen");
    if (!action)
        return;

    KToggleFullScreenAction *kAct = static_cast<KToggleFullScreenAction *>(action);
    kAct->setFullScreen(this, kAct->isChecked());
}


/********************************************************************************************************/

bool MainWin::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::WindowActivate: {
        BufferId bufferId = Client::bufferModel()->currentBuffer();
        if (bufferId.isValid())
            Client::instance()->markBufferAsRead(bufferId);
        break;
    }
    case QEvent::WindowDeactivate:
        if (bufferWidget()->autoMarkerLineOnLostFocus())
            bufferWidget()->setMarkerLine();
        break;
    default:
        break;
    }
    return QMainWindow::event(event);
}


void MainWin::moveEvent(QMoveEvent *event)
{
    if (!(windowState() & Qt::WindowMaximized))
        _normalPos = event->pos();

    QMainWindow::moveEvent(event);
}


void MainWin::resizeEvent(QResizeEvent *event)
{
    if (!(windowState() & Qt::WindowMaximized))
        _normalSize = event->size();

    QMainWindow::resizeEvent(event);
}


void MainWin::closeEvent(QCloseEvent *event)
{
    QtUiSettings s;
    QtUiApplication *app = qobject_cast<QtUiApplication *> qApp;
    Q_ASSERT(app);
    if (!app->isAboutToQuit() && QtUi::haveSystemTray() && s.value("MinimizeOnClose").toBool()) {
        QtUi::hideMainWidget();
        event->ignore();
    }
    else {
        event->accept();
        quit();
    }
}


void MainWin::messagesInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);

    bool hasFocus = QApplication::activeWindow() != 0;

    for (int i = start; i <= end; i++) {
        QModelIndex idx = Client::messageModel()->index(i, ChatLineModel::ContentsColumn);
        if (!idx.isValid()) {
            kDebug(300000) << "MainWin::messagesInserted(): Invalid model index!";
            continue;
        }
        Message::Flags flags = (Message::Flags)idx.data(ChatLineModel::FlagsRole).toInt();
        if (flags.testFlag(Message::Backlog) || flags.testFlag(Message::Self))
            continue;

        BufferId bufId = idx.data(ChatLineModel::BufferIdRole).value<BufferId>();
        BufferInfo::Type bufType = Client::networkModel()->bufferType(bufId);

        // check if bufferId belongs to the shown chatlists
        if (!(Client::bufferViewOverlay()->bufferIds().contains(bufId) ||
              Client::bufferViewOverlay()->tempRemovedBufferIds().contains(bufId)))
            continue;

        // check if it's the buffer currently displayed
        if (hasFocus && bufId == Client::bufferModel()->currentBuffer())
            continue;

        // only show notifications for higlights or queries
        if (bufType != BufferInfo::QueryBuffer && !(flags & Message::Highlight))
            continue;

        // and of course: don't notify for ignored messages
        if (Client::ignoreListManager() && Client::ignoreListManager()->match(idx.data(MessageModel::MessageRole).value<Message>(), Client::networkModel()->networkName(bufId)))
            continue;

        // seems like we have a legit notification candidate!
        QModelIndex senderIdx = Client::messageModel()->index(i, ChatLineModel::SenderColumn);
        QString sender = senderIdx.data(ChatLineModel::EditRole).toString();
        QString contents = idx.data(ChatLineModel::DisplayRole).toString();
        AbstractNotificationBackend::NotificationType type;

        if (bufType == BufferInfo::QueryBuffer && !hasFocus)
            type = AbstractNotificationBackend::PrivMsg;
        else if (bufType == BufferInfo::QueryBuffer && hasFocus)
            type = AbstractNotificationBackend::PrivMsgFocused;
        else if (flags & Message::Highlight && !hasFocus)
            type = AbstractNotificationBackend::Highlight;
        else
            type = AbstractNotificationBackend::HighlightFocused;

        QtUi::instance()->invokeNotification(bufId, type, sender, contents);
    }
}


void MainWin::currentBufferChanged(BufferId buffer)
{
    if (buffer.isValid())
        Client::instance()->markBufferAsRead(buffer);
}


void MainWin::clientNetworkCreated(NetworkId id)
{
    const Network *net = Client::network(id);
    QAction *act = new QAction(net->networkName(), this);
    act->setObjectName(QString("NetworkAction-%1").arg(id.toInt()));
    act->setData(QVariant::fromValue<NetworkId>(id));
    connect(net, SIGNAL(updatedRemotely()), this, SLOT(clientNetworkUpdated()));
    connect(act, SIGNAL(triggered()), this, SLOT(connectOrDisconnectFromNet()));

    QAction *beforeAction = 0;
    foreach(QAction *action, _networksMenu->actions()) {
        if (!action->data().isValid()) // ignore stock actions
            continue;
        if (net->networkName().localeAwareCompare(action->text()) < 0) {
            beforeAction = action;
            break;
        }
    }
    _networksMenu->insertAction(beforeAction, act);
}


void MainWin::clientNetworkUpdated()
{
    const Network *net = qobject_cast<const Network *>(sender());
    if (!net)
        return;

    QAction *action = findChild<QAction *>(QString("NetworkAction-%1").arg(net->networkId().toInt()));
    if (!action)
        return;

    action->setText(net->networkName());

    switch (net->connectionState()) {
    case Network::Initialized:
        action->setIcon(KIcon("network-connect"));
        // if we have no currently selected buffer, jump to the first connecting statusbuffer
        if (!bufferWidget()->currentBuffer().isValid()) {
            QModelIndex idx = Client::networkModel()->networkIndex(net->networkId());
            if (idx.isValid()) {
                BufferId statusBufferId = idx.data(NetworkModel::BufferIdRole).value<BufferId>();
                Client::bufferModel()->switchToBuffer(statusBufferId);
            }
        }
        break;
    case Network::Disconnected:
        action->setIcon(KIcon("network-disconnect"));
        break;
    default:
        action->setIcon(KIcon("network-wired"));
    }
}


void MainWin::clientNetworkRemoved(NetworkId id)
{
    QAction *action = findChild<QAction *>(QString("NetworkAction-%1").arg(id.toInt()));
    if (!action)
        return;

    action->deleteLater();
}


void MainWin::connectOrDisconnectFromNet()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (!act) return;
    const Network *net = Client::network(act->data().value<NetworkId>());
    if (!net) return;
    if (net->connectionState() == Network::Disconnected) net->requestConnect();
    else net->requestDisconnect();
}


void MainWin::on_jumpHotBuffer_triggered()
{
    if (!_bufferHotList->rowCount())
        return;

    Client::bufferModel()->switchToBuffer(_bufferHotList->hottestBuffer());
}


void MainWin::onJumpKey()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action || !Client::bufferModel())
        return;
    int idx = action->property("Index").toInt();

    if (_jumpKeyMap.isEmpty())
        _jumpKeyMap = CoreAccountSettings().jumpKeyMap();

    if (!_jumpKeyMap.contains(idx))
        return;

    BufferId buffer = _jumpKeyMap.value(idx);
    if (buffer.isValid())
        Client::bufferModel()->switchToBuffer(buffer);
}


void MainWin::bindJumpKey()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action || !Client::bufferModel())
        return;
    int idx = action->property("Index").toInt();

    _jumpKeyMap[idx] = Client::bufferModel()->currentBuffer();
    CoreAccountSettings().setJumpKeyMap(_jumpKeyMap);
}


void MainWin::on_actionDebugNetworkModel_triggered()
{
    QTreeView *view = new QTreeView;
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->setWindowTitle("Debug NetworkModel View");
    view->setModel(Client::networkModel());
    view->setColumnWidth(0, 250);
    view->setColumnWidth(1, 250);
    view->setColumnWidth(2, 80);
    view->resize(610, 300);
    view->show();
}


void MainWin::on_actionDebugHotList_triggered()
{
    _bufferHotList->invalidate();
    _bufferHotList->sort(0, Qt::DescendingOrder);

    QTreeView *view = new QTreeView;
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->setWindowTitle("Debug HotList View");
    view->setModel(_bufferHotList);
    view->show();
}


void MainWin::on_actionDebugBufferViewOverlay_triggered()
{
    DebugBufferViewOverlay *overlay = new DebugBufferViewOverlay(0);
    overlay->setAttribute(Qt::WA_DeleteOnClose);
    overlay->show();
}


void MainWin::on_actionDebugMessageModel_triggered()
{
    QTableView *view = new QTableView(0);
    DebugMessageModelFilter *filter = new DebugMessageModelFilter(view);
    filter->setSourceModel(Client::messageModel());
    view->setWindowTitle("Debug MessageModel View");
    view->setModel(filter);
    view->setAttribute(Qt::WA_DeleteOnClose, true);
    view->verticalHeader()->hide();
    view->horizontalHeader()->setStretchLastSection(true);
    view->show();
}


void MainWin::showStatusBarMessage(const QString &message)
{
    statusBar()->showMessage(message, 10000);
}
