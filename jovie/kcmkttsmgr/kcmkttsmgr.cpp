/***************************************************** vim:set ts=4 sw=4 sts=4:
  KControl module for KTTSD configuration and Job Management
  -------------------
  Copyright 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Copyright 2004-2005 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright 2009 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: Jeremy Whiting <jpwhiting@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/

// Note to programmers.  There is a subtle difference between a plugIn name and a
// synthesizer name.  The latter is a translated name, for example, "Festival Interactivo",
// while the former is alway an English name, example "Festival Interactive".

// KCMKttsMgr includes.
#include "kcmkttsmgr.h"
#include "moc_kcmkttsmgr.cpp"

// C++ includes.
#include <math.h>

// Qt includes.
#include <QtGui/QWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QLabel>
#include <QtGui/QTreeWidget>
#include <QtGui/QHeaderView>
#include <QtCore/QTextStream>
#include <QtGui/QMenu>

// KDE includes.
#include <klineedit.h>
#include <kurlrequester.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <ktoolinvocation.h>
#include <kdialog.h>
#include <kspeech.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kservicetypetrader.h>
#include <kpluginloader.h>

// KTTS includes.
#include "talkercode.h"
#include "filterconf.h"
#include "kttsjobmgr.h"

// Some constants.
// Defaults set when clicking Defaults button.
const bool autostartMgrCheckBoxValue = true;
const bool autoexitMgrCheckBoxValue = true;


// Make this a plug in.
K_PLUGIN_FACTORY (KCMKttsMgrFactory, registerPlugin<KCMKttsMgr>();)
K_EXPORT_PLUGIN (KCMKttsMgrFactory ("jovie"))


// ----------------------------------------------------------------------------

FilterListModel::FilterListModel (FilterList filters, QObject *parent)
        : QAbstractListModel (parent), m_filters (filters)
{
}

int FilterListModel::rowCount (const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_filters.count();
    else
        return 0;
}

int FilterListModel::columnCount (const QModelIndex &parent) const
{
    Q_UNUSED (parent);
    return 2;
}

QModelIndex FilterListModel::index (int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex (row, column, 0);
    else
        return QModelIndex();
}

QModelIndex FilterListModel::parent (const QModelIndex &index) const
{
    Q_UNUSED (index);
    return QModelIndex();
}

QVariant FilterListModel::data (const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= m_filters.count())
        return QVariant();

    if (index.column() < 0 || index.column() >= 2)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        switch (index.column()) {
        case 0:
            return QVariant();
            break;
        case 1:
            return m_filters.at (index.row()).userFilterName;
            break;
        }

    if (role == Qt::CheckStateRole)
        switch (index.column()) {
        case 0:
            if (m_filters.at (index.row()).enabled)
                return Qt::Checked;
            else
                return Qt::Unchecked;
            break;
        case 1:
            return QVariant();
            break;
        }

    return QVariant();
}

Qt::ItemFlags FilterListModel::flags (const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    switch (index.column()) {
    case 0:
        return QAbstractItemModel::flags (index) | Qt::ItemIsEnabled |
               Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        break;
    case 1:
        return QAbstractItemModel::flags (index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        break;
    }
    return QAbstractItemModel::flags (index) | Qt::ItemIsEnabled;
}

QVariant FilterListModel::headerData (int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        switch (section) {
        case 0:
            return QLatin1String( "" );
        case 1:
            return i18n ("Filter");
        };
    return QVariant();
}

bool FilterListModel::removeRow (int row, const QModelIndex & parent)
{
    beginRemoveRows (parent, row, row);
    m_filters.removeAt (row);
    endRemoveRows();
    return true;
}

FilterItem FilterListModel::getRow (int row) const
{
    if (row < 0 || row >= rowCount()) return FilterItem();
    return m_filters[row];
}

bool FilterListModel::appendRow (FilterItem& filter)
{
    beginInsertRows (QModelIndex(), m_filters.count(), m_filters.count());
    m_filters.append (filter);
    endInsertRows();
    return true;
}

bool FilterListModel::updateRow (int row, FilterItem& filter)
{
    m_filters.replace (row, filter);
    emit dataChanged (index (row, 0, QModelIndex()), index (row, columnCount() - 1, QModelIndex()));
    return true;
}

bool FilterListModel::swap (int i, int j)
{
    m_filters.swap (i, j);
    emit dataChanged (index (i, 0, QModelIndex()), index (j, columnCount() - 1, QModelIndex()));
    return true;
}

void FilterListModel::clear()
{
    m_filters.clear();
    emit reset();
}

// ----------------------------------------------------------------------------

/**
* Constructor.
*/
KCMKttsMgr::KCMKttsMgr (QWidget *parent, const QVariantList &) :
        KCModule (KCMKttsMgrFactory::componentData(), parent/*, name*/),
        m_kspeech (0)
{

    // kDebug() << "KCMKttsMgr constructor running.";

    // Initialize some variables.
    m_config = 0;
    m_jobMgrWidget = 0;
    m_configDlg = 0;
    m_changed = false;
    m_suppressConfigChanged = false;

    // Add the KTTS Manager widget
    setupUi (this);

    // Connect Views to Models and set row selection mode.
    talkersView->setModel (&m_talkerListModel);
    filtersView->setModel (&m_filterListModel);
    talkersView->setSelectionBehavior (QAbstractItemView::SelectRows);
    filtersView->setSelectionBehavior (QAbstractItemView::SelectRows);
    talkersView->setRootIsDecorated (false);
    filtersView->setRootIsDecorated (false);
    talkersView->setItemsExpandable (false);
    filtersView->setItemsExpandable (false);

    // Give buttons icons.
    // Talkers tab.
    higherTalkerPriorityButton->setIcon (KIcon ( QLatin1String( "go-up" )));
    lowerTalkerPriorityButton->setIcon (KIcon ( QLatin1String( "go-down" )));
    removeTalkerButton->setIcon (KIcon ( QLatin1String( "user-trash" )));
    configureTalkerButton->setIcon (KIcon ( QLatin1String( "configure" )));

    // Filters tab.
    higherFilterPriorityButton->setIcon (KIcon ( QLatin1String( "go-up" )));
    lowerFilterPriorityButton->setIcon (KIcon ( QLatin1String( "go-down" )));
    removeFilterButton->setIcon (KIcon ( QLatin1String( "user-trash" )));
    configureFilterButton->setIcon (KIcon ( QLatin1String( "configure" )));

    // Object for the KTTSD configuration.
    m_config = new KConfig (QLatin1String( "kttsdrc" ));

    // Connect the signals from the KCMKtssMgrWidget to this class.

    // General tab.
    connect (enableJovieCheckBox, SIGNAL (toggled(bool)),
             SLOT (slotEnableJovie_toggled(bool)));

    // Talker tab.
    connect (addTalkerButton, SIGNAL (clicked()),
             this, SLOT (slotAddTalkerButton_clicked()));
    connect (higherTalkerPriorityButton, SIGNAL (clicked()),
             this, SLOT (slotHigherTalkerPriorityButton_clicked()));
    connect (lowerTalkerPriorityButton, SIGNAL (clicked()),
             this, SLOT (slotLowerTalkerPriorityButton_clicked()));
    connect (removeTalkerButton, SIGNAL (clicked()),
             this, SLOT (slotRemoveTalkerButton_clicked()));
    connect (configureTalkerButton, SIGNAL (clicked()),
             this, SLOT (slotConfigureTalkerButton_clicked()));
    connect (talkersView, SIGNAL (clicked(QModelIndex)),
             this, SLOT (updateTalkerButtons()));

    // Filter tab.
    connect (addFilterButton, SIGNAL (clicked()),
             this, SLOT (slotAddFilterButton_clicked()));
    connect (higherFilterPriorityButton, SIGNAL (clicked()),
             this, SLOT (slotHigherFilterPriorityButton_clicked()));
    connect (lowerFilterPriorityButton, SIGNAL (clicked()),
             this, SLOT (slotLowerFilterPriorityButton_clicked()));
    connect (removeFilterButton, SIGNAL (clicked()),
             this, SLOT (slotRemoveFilterButton_clicked()));
    connect (configureFilterButton, SIGNAL (clicked()),
             this, SLOT (slotConfigureFilterButton_clicked()));
    connect (filtersView, SIGNAL (clicked(QModelIndex)),
             this, SLOT (updateFilterButtons()));
    connect (filtersView, SIGNAL (clicked(QModelIndex)),
             this, SLOT (slotFilterListView_clicked(QModelIndex)));


    // Others.
    connect (mainTab, SIGNAL (currentChanged(int)),
             this, SLOT (slotTabChanged()));

    // See if Jovie is already running, and if so, create jobs tab.
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered (QLatin1String( "org.kde.jovie" )))
        jovieStarted();
    else
        // Start Jovie if check box is checked.
        slotEnableJovie_toggled (enableJovieCheckBox->isChecked());

    // Adjust view column sizes.
    // TODO: To work properly, this needs to be done after the widgets are shown.
    // Possibly in Resize event?
    for (int i = 0; i < m_filterListModel.columnCount(); ++i)
        filtersView->resizeColumnToContents (i);
    for (int i = 0; i < m_talkerListModel.columnCount(); ++i)
        talkersView->resizeColumnToContents (i);

    // Switch to Talkers tab if none configured,
    // otherwise switch to Jobs tab if it is active.
    if (m_talkerListModel.rowCount() == 0)
        mainTab->setCurrentIndex (wpTalkers);
    else if (enableJovieCheckBox->isChecked())
        mainTab->setCurrentIndex (wpJobs);
}

/**
* Destructor.
*/
KCMKttsMgr::~KCMKttsMgr()
{
    // kDebug() << "KCMKttsMgr::~KCMKttsMgr: Running";
    delete m_config;
}

/**
* This method is invoked whenever the module should read its
* configuration (most of the times from a config file) and update the
* user interface. This happens when the user clicks the "Reset" button in
* the control center, to undo all of his changes and restore the currently
* valid settings. NOTE that this is not called after the modules is loaded,
* so you probably want to call this method in the constructor.
*/
void KCMKttsMgr::load()
{
    // kDebug() << "KCMKttsMgr::load: Running";

    m_changed = false;
    // Don't emit changed() signal while loading.
    m_suppressConfigChanged = true;

    // Set the group general for the configuration of kttsd itself (no plug ins)
    KConfigGroup generalConfig (m_config, "General");

    // Overall settings.
    enableJovieCheckBox->setChecked (generalConfig.readEntry ("EnableJovie",
                                     enableJovieCheckBox->isChecked()));

    // Last filter ID.  Used to generate a new ID for an added filter.
    m_lastFilterID = 0;

    // Load existing Talkers into Talker List.
    m_talkerListModel.loadTalkerCodesFromConfig (m_config);

    // Last talker ID.  Used to generate a new ID for an added talker.
    //m_lastTalkerID = m_talkerListModel.highestTalkerId();

    // Dictionary mapping languages to language codes.
    m_languagesToCodes.clear();
    for (int i = 0; i < m_talkerListModel.rowCount(); ++i) {
        QString language = m_talkerListModel.getRow (i).language();
        m_languagesToCodes[language] = language;
    }

    // Iterate thru the possible plug ins getting their language support codes.
    //for(int i=0; i < offers.count() ; ++i)
    //{
    //    QString synthName = offers[i]->name();
    //    QStringList languageCodes = offers[i]->property("X-KDE-Languages").toStringList();
    //    // Add language codes to the language-to-language code map.
    //    QStringList::ConstIterator endLanguages(languageCodes.constEnd());
    //    for( QStringList::ConstIterator it = languageCodes.constBegin(); it != endLanguages; ++it )
    //    {
    //        QString language = TalkerCode::languageCodeToLanguage(*it);
    //        m_languagesToCodes[language] = *it;
    //    }

    //    // All plugins support "Other".
    //    // TODO: Eventually, this should not be necessary, since all plugins will know
    //    // the languages they support and report them in call to getSupportedLanguages().
    //    if (!languageCodes.contains("other")) languageCodes.append("other");

    //    // Add supported language codes to synthesizer-to-language map.
    //    m_synthToLangMap[synthName] = languageCodes;
    //}

    // Add "Other" language.
    //m_languagesToCodes[i18nc("Other language", "Other")] = "other";

    // Load Filters.
    m_filterListModel.clear();
    QStringList filterIDsList = generalConfig.readEntry ("FilterIDs", QStringList());
    // kDebug() << "KCMKttsMgr::load: FilterIDs = " << filterIDsList;
    if (!filterIDsList.isEmpty()) {
        QStringList::ConstIterator itEnd = filterIDsList.constEnd();
        for (QStringList::ConstIterator it = filterIDsList.constBegin(); it != itEnd; ++it) {
            QString filterID = *it;
            // kDebug() << "KCMKttsMgr::load: filterID = " << filterID;
            KConfigGroup filterConfig (m_config, QLatin1String( "Filter_" ) + filterID);
            QString desktopEntryName = filterConfig.readEntry ("DesktopEntryName", QString());
            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            QString filterPlugInName;
            filterPlugInName = FilterDesktopEntryNameToName (desktopEntryName);
            if (!filterPlugInName.isEmpty()) {
                FilterItem fi;
                fi.id = filterID;
                fi.plugInName = filterPlugInName;
                fi.desktopEntryName = desktopEntryName;
                fi.userFilterName = filterConfig.readEntry ("UserFilterName", filterPlugInName);
                fi.multiInstance = filterConfig.readEntry ("MultiInstance", false);
                fi.enabled = filterConfig.readEntry ("Enabled", false);
                // Determine if this filter is a Sentence Boundary Detector (SBD).
                m_filterListModel.appendRow (fi);
                if (filterID.toInt() > m_lastFilterID) m_lastFilterID = filterID.toInt();
            }
        }
    }

    // Add at least one unchecked instance of each available filter plugin if there is
    // not already at least one instance and the filter can autoconfig itself.
    //offers = KServiceTypeTrader::self()->query("KTTSD/FilterPlugin");
    //for (int i=0; i < offers.count() ; ++i)
    //{
    //    QString filterPlugInName = offers[i]->name();
    //    QString desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
    //    if (!desktopEntryName.isEmpty() && (countFilterPlugins(filterPlugInName) == 0))
    //    {
    //        // Must load plugin to determine if it supports multiple instances
    //        // and to see if it can autoconfigure itself.
    //        KttsFilterConf* filterPlugIn = loadFilterPlugin(desktopEntryName);
    //        if (filterPlugIn)
    //        {
    //            ++m_lastFilterID;
    //            QString filterID = QString::number(m_lastFilterID);
    //            QString groupName = "Filter_" + filterID;
    //            filterPlugIn->load(m_config, groupName);
    //            QString userFilterName = filterPlugIn->userPlugInName();
    //            if (!userFilterName.isEmpty())
    //            {
    //                kDebug() << "KCMKttsMgr::load: auto-configuring filter " << userFilterName;
    //                bool multiInstance = filterPlugIn->supportsMultiInstance();
    //                FilterItem fi;
    //                fi.id = filterID;
    //                fi.userFilterName = userFilterName;
    //                fi.plugInName = filterPlugInName;
    //                fi.desktopEntryName = desktopEntryName;
    //                fi.enabled = true;
    //                fi.multiInstance = multiInstance;
    //                // Determine if plugin is an SBD filter.
    //                bool isSbd = filterPlugIn->isSBD();
    //                if (isSbd)
    //                    m_sbdFilterListModel.appendRow(fi);
    //                else
    //                    m_filterListModel.appendRow(fi);
    //                KConfigGroup filterConfig(m_config, groupName);
    //                filterPlugIn->save(m_config, groupName);
    //                filterConfig.writeEntry("DesktopEntryName", desktopEntryName);
    //                filterConfig.writeEntry("UserFilterName", userFilterName);
    //                filterConfig.writeEntry("Enabled", isSbd);
    //                filterConfig.writeEntry("MultiInstance", multiInstance);
    //                filterConfig.writeEntry("IsSBD", isSbd);
    //                filterIDsList.append(filterID);
    //            } else m_lastFilterID--;
    //        } else
    //            kDebug() << "KCMKttsMgr::load: Unable to load filter plugin " << filterPlugInName
    //                << " DesktopEntryName " << desktopEntryName << endl;
    //        delete filterPlugIn;
    //    }
    //}
    // Rewrite list of FilterIDs in case we added any.
    QString filterIDs = filterIDsList.join (QLatin1String( "," ));
    generalConfig.writeEntry ("FilterIDs", filterIDs);
    m_config->sync();

    // Update controls based on new states.
    updateTalkerButtons();
    updateFilterButtons();

    m_changed = false;
    m_suppressConfigChanged = false;
}

/**
* This function gets called when the user wants to save the settings in
* the user interface, updating the config files or wherever the
* configuration is stored. The method is called when the user clicks "Apply"
* or "Ok".
*/
void KCMKttsMgr::save()
{
    // kDebug() << "KCMKttsMgr::save: Running";
    m_changed = false;

    // Clean up config.
    m_config->deleteGroup ("General", 0);

    // Set the group general for the configuration of kttsd itself (no plug ins)
    KConfigGroup generalConfig (m_config, "General");

    // Uncheck and disable KTTSD checkbox if no Talkers are configured.
    // Enable checkbox if at least one Talker is configured.
    bool enableJovieWasToggled = false;
    //if (m_talkerListModel.rowCount() == 0)
    //{
    //enableKttsdWasToggled = enableKttsdCheckBox->isChecked();
    //enableKttsdCheckBox->setChecked(false);
    //enableKttsdCheckBox->setEnabled(false);
    // Might as well zero LastTalkerID as well.
    //m_lastTalkerID = 0;
    //}
    //else
    enableJovieCheckBox->setEnabled (true);

    generalConfig.writeEntry ("EnableKttsd", enableJovieCheckBox->isChecked());

    // Get ordered list of all talker IDs.
    QList<TalkerCode> talkers;
    QStringList talkerIDsList;
    KConfigGroup talkerGroup (m_config, "Talkers");
    talkerGroup.deleteGroup();
    for (int i = 0; i < m_talkerListModel.rowCount(); ++i) {
        TalkerCode talker = m_talkerListModel.getRow (i);
        talkers << talker;
        talkerGroup.writeEntry (talker.name(), talker.getTalkerCode());
        talkerIDsList << talker.name();
    }

    QString talkerIDs = talkerIDsList.join (QLatin1String( "," ));
    generalConfig.writeEntry ("TalkerIDs", talkerIDs);

    // Erase obsolete Talker_nn sections.
    QStringList groupList = m_config->groupList();
    int groupListCount = groupList.count();
    for (int groupNdx = 0; groupNdx < groupListCount; ++groupNdx) {
        QString groupName = groupList[groupNdx];
        if (groupName.left (7) == QLatin1String( "Talker_" )) {
            QString groupTalkerID = groupName.mid (7);
            if (!talkerIDsList.contains (groupTalkerID))
                m_config->deleteGroup (groupName, 0);
        }
    }

    // Get ordered list of all filter IDs.  Record enabled state of each filter.
    QStringList filterIDsList;
    for (int i = 0; i < m_filterListModel.rowCount(); ++i) {
        FilterItem fi = m_filterListModel.getRow (i);
        filterIDsList.append (fi.id);
        KConfigGroup filterConfig (m_config, QLatin1String( "Filter_" ) + fi.id);
        filterConfig.writeEntry ("Enabled", fi.enabled);
    }
    QString filterIDs = filterIDsList.join (QLatin1String( "," ));
    generalConfig.writeEntry ("FilterIDs", filterIDs);

    // Erase obsolete Filter_nn sections.
    for (int groupNdx = 0; groupNdx < groupListCount; ++groupNdx) {
        QString groupName = groupList[groupNdx];
        if (groupName.left (7) == QLatin1String( "Filter_" )) {
            QString groupFilterID = groupName.mid (7);
            if (!filterIDsList.contains (groupFilterID))
                m_config->deleteGroup (groupName, 0);
        }
    }

    m_config->sync();

    // apply changes in the jobs page if it exists
    if (m_jobMgrWidget) {
        m_jobMgrWidget->save();
    }

    // If we automatically unchecked the Enable KTTSD checkbox, stop KTTSD.
    if (enableJovieWasToggled)
        slotEnableJovie_toggled (false);
    else {
        // If Jovie is running, reinitialize it.
        if (m_kspeech) {
            kDebug() << "Restarting Jovie";
            m_kspeech->reinit();
        }
    }
}

void KCMKttsMgr::slotTabChanged()
{
    // TODO: Commentting this out to avoid a crash.  It seems there is a bug in
    // KDialog.  The workaround is to call setDefaultButton(), but that method
    // is not available to a KCModule.  Uncomment this when bug is fixed.
    // setButtons(buttons());
    int currentPageIndex = mainTab->currentIndex();
    if (currentPageIndex == wpJobs) {
        if (m_changed) {
            KMessageBox::information (this,
                                      i18n ("You have made changes to the configuration but have not saved them yet.  "
                                            "Click Apply to save the changes or Cancel to abandon the changes."));
        }
    }
}

/**
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The
* default values should probably be the same as the ones the application
* uses when started without a config file.
*/
void KCMKttsMgr::defaults()
{
    // kDebug() << "Running: KCMKttsMgr::defaults: Running";

    int currentPageIndex = mainTab->currentIndex();
    // configChanged();
}

/**
* This is a static method which gets called to realize the modules settings
* during the startup of KDE. NOTE that most modules do not implement this
* method, but modules like the keyboard and mouse modules, which directly
* interact with the X-server, need this method. As this method is static,
* it can avoid to create an instance of the user interface, which is often
* not needed in this case.
*/
void KCMKttsMgr::init()
{
    // kDebug() << "KCMKttsMgr::init: Running";
}

/**
* This function returns the small quickhelp.
* That is displayed in the sidebar in the KControl
*/
QString KCMKttsMgr::quickHelp() const
{
    // kDebug() << "KCMKttsMgr::quickHelp: Running";
    return i18n (
               "<h1>Text-to-Speech</h1>"
               "<p>This is the configuration for the text-to-speech D-Bus service</p>"
               "<p>This allows other applications to access text-to-speech resources</p>"
               "<p>Be sure to configure a default language for the language you are using as this will be the language used by most of the applications</p>");
}

const KAboutData* KCMKttsMgr::aboutData() const
{
    KAboutData *about =
        new KAboutData (I18N_NOOP ("jovie"), 0, ki18n ("KCMKttsMgr"),
                        0, KLocalizedString(), KAboutData::License_GPL,
                        ki18n ("(c) 2010, Jeremy Whiting"));

    about->addAuthor (ki18n ("José Pablo Ezequiel Fernández"), ki18n ("Author") , "pupeno@kde.org");
    about->addAuthor (ki18n ("Gary Cramblitt"), ki18n ("Maintainer") , "garycramblitt@comcast.net");
    about->addAuthor (ki18n ("Olaf Schmidt"), ki18n ("Contributor"), "ojschmidt@kde.org");
    about->addAuthor (ki18n ("Paul Giannaros"), ki18n ("Contributor"), "ceruleanblaze@gmail.com");

    return about;
}

/**
 * Loads the configuration plug in for a named filter plug in.
 * @param plugInName       DesktopEntryName of the plugin.
 * @return                 Pointer to the configuration plugin for the Filter.
 */
KttsFilterConf* KCMKttsMgr::loadFilterPlugin (const QString& plugInName)
{
    // kDebug() << "KCMKttsMgr::loadPlugin: Running";

    // Find the plugin.
    KService::List offers = KServiceTypeTrader::self()->query (QLatin1String( "Jovie/FilterPlugin" ),
                            QString (QLatin1String( "DesktopEntryName == '%1'" )).arg (plugInName));

    if (offers.count() == 1) {
        // When the entry is found, load the plug in
        // First create a factory for the library
        KPluginFactory *factory = KPluginLoader(QLatin1String( offers[0]->library().toLatin1() )).factory();
        if (factory) {
            // If the factory is created successfully, instantiate the KttsFilterConf class for the
            // specific plug in to get the plug in configuration object.
            KttsFilterConf *plugIn = factory->create<KttsFilterConf> ();
            if (plugIn) {
                // If everything went ok, return the plug in pointer.
                return plugIn;
            } else {
                // Something went wrong, returning null.
                kDebug() << "KCMKttsMgr::loadFilterPlugin: Unable to instantiate KttsFilterConf class for plugin " << plugInName;
                return NULL;
            }
        } else {
            // Something went wrong, returning null.
            kDebug() << "KCMKttsMgr::loadFilterPlugin: Unable to create Factory object for plugin " << plugInName;
            return NULL;
        }
    }
    // The plug in was not found (unexpected behaviour, returns null).
    kDebug() << "KCMKttsMgr::loadFilterPlugin: KTrader did not return an offer for plugin " << plugInName;
    return NULL;
}

/**
 * Add a talker.
 */
void KCMKttsMgr::slotAddTalkerButton_clicked()
{
    QPointer<AddTalker> dlg = new AddTalker (this);
    if (dlg->exec() == QDialog::Accepted) {
        TalkerCode code = dlg->getTalkerCode();

        // Add to list of Talkers.
        m_talkerListModel.appendRow (code);

        // Make sure visible.
        const QModelIndex modelIndex = m_talkerListModel.index (m_talkerListModel.rowCount() - 1,
                                       0, QModelIndex());
        talkersView->scrollTo (modelIndex);

        // Select the new item, update buttons.
        talkersView->setCurrentIndex (modelIndex);
        updateTalkerButtons();

        // Inform Control Center that change has been made.
        configChanged();
    }
    delete dlg;

    kDebug() << "KCMKttsMgr::addTalker: done.";
}

void KCMKttsMgr::slotAddFilterButton_clicked()
{
    addFilter();
}

/**
* Add a filter.
*/
void KCMKttsMgr::addFilter()
{
    QTreeView* lView = filtersView;
    FilterListModel* model = qobject_cast<FilterListModel *> (lView->model());

    // Build a list of filters that support multiple instances and let user choose.
    QStringList filterPlugInNames;
    for (int i = 0; i < model->rowCount(); ++i) {
        FilterItem fi = model->getRow (i);
        if (fi.multiInstance) {
            if (!filterPlugInNames.contains (fi.plugInName))
                filterPlugInNames.append (fi.plugInName);
        }
    }
    // Append those available plugins not yet in the list at all.
    KService::List offers = KServiceTypeTrader::self()->query (QLatin1String( "Jovie/FilterPlugin" ));
    for (int i = 0; i < offers.count() ; ++i) {
        QString filterPlugInName = offers[i]->name();
        if (countFilterPlugins (filterPlugInName) == 0) {
            QString desktopEntryName = FilterNameToDesktopEntryName (filterPlugInName);
            KttsFilterConf* filterConf = loadFilterPlugin (desktopEntryName);
            if (filterConf) {
                filterPlugInNames.append (filterPlugInName);
                delete filterConf;
            }
        }
    }

    // If no choice (shouldn't happen), bail out.
    // kDebug() << "KCMKttsMgr::addFilter: filterPluginNames = " << filterPlugInNames;
    if (filterPlugInNames.count() == 0) return;

    // If exactly one choice, skip selection dialog, otherwise display list to user to select from.
    bool okChosen = false;
    QString filterPlugInName;
    if (filterPlugInNames.count() > 1) {
        filterPlugInName = KInputDialog::getItem (
                               i18n ("Select Filter"),
                               i18n ("Filter"),
                               filterPlugInNames,
                               0,
                               false,
                               &okChosen,
                               this);
        if (!okChosen) return;
    } else
        filterPlugInName = filterPlugInNames[0];

    // kDebug() << "KCMKttsMgr::addFilter: filterPlugInName = " << filterPlugInName;

    // Assign a new Filter ID for the filter.  Wraps around to 1.
    QString filterID = QString::number (m_lastFilterID + 1);

    // Erase extraneous Filter configuration entries that might be there.
    m_config->deleteGroup (QLatin1String ("Filter_") + filterID, 0);
    m_config->sync();

    // Get DesktopEntryName from the translated name.
    QString desktopEntryName = FilterNameToDesktopEntryName (filterPlugInName);
    // This shouldn't happen, but just in case.
    if (desktopEntryName.isEmpty()) return;

    // Load the plugin.
    m_loadedFilterPlugIn = loadFilterPlugin (desktopEntryName);
    if (!m_loadedFilterPlugIn) return;

    // Permit plugin to autoconfigure itself.
    m_loadedFilterPlugIn->load (m_config, QLatin1String ("Filter_") + filterID);

    // Display configuration dialog for user to configure the plugin.
    configureFilter();

    // Did user Cancel?
    if (!m_loadedFilterPlugIn) {
        delete m_configDlg;
        m_configDlg = 0;
        return;
    }

    // Get user's name for Filter.
    QString userFilterName = m_loadedFilterPlugIn->userPlugInName();

    // If user properly configured the plugin, save its configuration.
    if (!userFilterName.isEmpty()) {
        // Let plugin save its configuration.
        m_loadedFilterPlugIn->save (m_config, QLatin1String ("Filter_" ) + filterID);

        // Record last Filter ID used for next add.
        m_lastFilterID = filterID.toInt();

        // Determine if filter supports multiple instances.
        bool multiInstance = m_loadedFilterPlugIn->supportsMultiInstance();

        // Record configuration data.  Note, might as well do this now.
        KConfigGroup filterConfig (m_config, QLatin1String ("Filter_" ) + filterID);
        filterConfig.writeEntry ("DesktopEntryName", desktopEntryName);
        filterConfig.writeEntry ("UserFilterName", userFilterName);
        filterConfig.writeEntry ("MultiInstance", multiInstance);
        filterConfig.writeEntry ("Enabled", true);
        m_config->sync();

        // Add listview item.
        FilterItem fi;
        fi.id = filterID;
        fi.plugInName = filterPlugInName;
        fi.userFilterName = userFilterName;
        fi.desktopEntryName = desktopEntryName;
        fi.multiInstance = multiInstance;
        fi.enabled = true;
        model->appendRow (fi);

        // Make sure visible.
        QModelIndex modelIndex = model->index (model->rowCount() - 1, 0, QModelIndex());
        lView->scrollTo (modelIndex);

        // Select the new item, update buttons.
        lView->setCurrentIndex (modelIndex);
        updateFilterButtons();

        // Inform Control Center that change has been made.
        configChanged();
    }

    // Don't need plugin in memory anymore.
    delete m_loadedFilterPlugIn;
    m_loadedFilterPlugIn = 0;
    delete m_configDlg;
    m_configDlg = 0;

    // kDebug() << "KCMKttsMgr::addFilter: done.";
}

/**
* Remove talker.
*/
void KCMKttsMgr::slotRemoveTalkerButton_clicked()
{
    // kDebug() << "KCMKttsMgr::removeTalker: Running";

    // Get the selected talker.
    QModelIndex modelIndex = talkersView->currentIndex();
    if (!modelIndex.isValid()) return;

    // Delete the talker from configuration file?
    QString talkerID = m_talkerListModel.getRow (modelIndex.row()).name();
    m_config->deleteGroup (QLatin1String ("Talker_") + talkerID, 0);

    // Delete the talker from the list of Talkers.
    m_talkerListModel.removeRow (modelIndex.row());

    updateTalkerButtons();

    // Emit configuration changed.
    configChanged();
}

void KCMKttsMgr::slotRemoveFilterButton_clicked()
{
    removeFilter();
}

/**
* Remove filter.
*/
void KCMKttsMgr::removeFilter()
{
    // kDebug() << "KCMKttsMgr::removeFilter: Running";

    FilterListModel* model;
    QTreeView* lView = filtersView;
    model = qobject_cast<FilterListModel *> (lView->model());
    QModelIndex modelIndex = lView->currentIndex();
    if (!modelIndex.isValid()) return;
    QString filterID = model->getRow (modelIndex.row()).id;
    // Delete the filter from list view.
    model->removeRow (modelIndex.row());
    updateFilterButtons();

    // Delete the filter from the configuration file?
    kDebug() << "KCMKttsMgr::removeFilter: removing FilterID = " << filterID << " from config file.";
    m_config->deleteGroup (QLatin1String ("Filter_") + filterID, 0);

    // Emit configuration changed.
    configChanged();
}

void KCMKttsMgr::slotHigherTalkerPriorityButton_clicked()
{
    QModelIndex modelIndex = talkersView->currentIndex();
    if (!modelIndex.isValid()) return;
    m_talkerListModel.swap (modelIndex.row(), modelIndex.row() - 1);
    modelIndex = m_talkerListModel.index (modelIndex.row() - 1, 0, QModelIndex());
    talkersView->scrollTo (modelIndex);
    talkersView->setCurrentIndex (modelIndex);
    updateTalkerButtons();
    configChanged();
}

void KCMKttsMgr::slotHigherFilterPriorityButton_clicked()
{
    QModelIndex modelIndex = filtersView->currentIndex();
    if (!modelIndex.isValid()) return;
    m_filterListModel.swap (modelIndex.row(), modelIndex.row() - 1);
    modelIndex = m_filterListModel.index (modelIndex.row() - 1, 0, QModelIndex());
    filtersView->scrollTo (modelIndex);
    filtersView->setCurrentIndex (modelIndex);
    updateFilterButtons();
    configChanged();
}

void KCMKttsMgr::slotLowerTalkerPriorityButton_clicked()
{
    QModelIndex modelIndex = talkersView->currentIndex();
    if (!modelIndex.isValid()) return;
    m_talkerListModel.swap (modelIndex.row(), modelIndex.row() + 1);
    modelIndex = m_talkerListModel.index (modelIndex.row() + 1, 0, QModelIndex());
    talkersView->scrollTo (modelIndex);
    talkersView->setCurrentIndex (modelIndex);
    updateTalkerButtons();
    configChanged();
}

void KCMKttsMgr::slotLowerFilterPriorityButton_clicked()
{
    QModelIndex modelIndex = filtersView->currentIndex();
    if (!modelIndex.isValid()) return;
    m_filterListModel.swap (modelIndex.row(), modelIndex.row() + 1);
    modelIndex = m_filterListModel.index (modelIndex.row() + 1, 0, QModelIndex());
    filtersView->scrollTo (modelIndex);
    filtersView->setCurrentIndex (modelIndex);
    updateFilterButtons();
    configChanged();
}

/**
* Update the status of the Talker buttons.
*/
void KCMKttsMgr::updateTalkerButtons()
{
    // kDebug() << "KCMKttsMgr::updateTalkerButtons: Running";
    QModelIndex modelIndex = talkersView->currentIndex();
    if (modelIndex.isValid()) {
        removeTalkerButton->setEnabled (true);
        configureTalkerButton->setEnabled (true);
        higherTalkerPriorityButton->setEnabled (modelIndex.row() != 0);
        lowerTalkerPriorityButton->setEnabled (modelIndex.row() < (m_talkerListModel.rowCount() - 1));
    } else {
        removeTalkerButton->setEnabled (false);
        configureTalkerButton->setEnabled (false);
        higherTalkerPriorityButton->setEnabled (false);
        lowerTalkerPriorityButton->setEnabled (false);
    }
    // kDebug() << "KCMKttsMgr::updateTalkerButtons: Exiting";
}

/**
* Update the status of the normal Filter buttons.
*/
void KCMKttsMgr::updateFilterButtons()
{
    // kDebug() << "KCMKttsMgr::updateFilterButtons: Running";
    QModelIndex modelIndex = filtersView->currentIndex();
    if (modelIndex.isValid()) {
        removeFilterButton->setEnabled (true);
        configureFilterButton->setEnabled (true);
        higherFilterPriorityButton->setEnabled (modelIndex.row() != 0);
        lowerFilterPriorityButton->setEnabled (modelIndex.row() < (m_filterListModel.rowCount() - 1));
    } else {
        removeFilterButton->setEnabled (false);
        configureFilterButton->setEnabled (false);
        higherFilterPriorityButton->setEnabled (false);
        lowerFilterPriorityButton->setEnabled (false);
    }
    // kDebug() << "KCMKttsMgr::updateFilterButtons: Exiting";
}

/**
* This signal is emitted whenever user checks/unchecks the Enable TTS System check box.
*/
void KCMKttsMgr::slotEnableJovie_toggled (bool)
{
    // Prevent re-entrancy.
    static bool reenter;
    if (reenter) return;
    reenter = true;
    // See if Jovie is running.
    bool kttsdRunning = (QDBusConnection::sessionBus().interface()->isServiceRegistered (QLatin1String( "org.kde.jovie" )));

    // kDebug() << "KCMKttsMgr::slotEnableKttsd_toggled: kttsdRunning = " << kttsdRunning;
    // If Enable Jovie check box is checked and it is not running, then start Jovie.
    if (enableJovieCheckBox->isChecked()) {
        if (!kttsdRunning) {
            // kDebug() << "KCMKttsMgr::slotEnableKttsd_toggled:: Starting Jovie";
            QString error;
            if (KToolInvocation::startServiceByDesktopName (QLatin1String( "jovie" ), QStringList(), &error)) {
                kDebug() << "Starting Jovie failed with message " << error;
                enableJovieCheckBox->setChecked (false);

            } else {
                configChanged();
                jovieStarted();
            }
        }
    } else
        // If check box is not checked and it is running, then stop Jovie.
    {
        if (kttsdRunning) {
            // kDebug() << "KCMKttsMgr::slotEnableKttsd_toggled:: Stopping Jovie";
            if (!m_kspeech)
                m_kspeech = new OrgKdeKSpeechInterface (QLatin1String( "org.kde.jovie" ), QLatin1String( "/KSpeech" ), QDBusConnection::sessionBus());
            m_kspeech->kttsdExit();
            delete m_kspeech;
            m_kspeech = 0;
            configChanged();
        }
    }
    reenter = false;
}

/**
* This slot is called whenever KTTSD starts or restarts.
*/
void KCMKttsMgr::jovieStarted()
{
    // kDebug() << "KCMKttsMgr::jovieStarted: Running";
    bool kttsdLoaded = (m_jobMgrWidget != 0);
    // Load Job Manager Part library.
    if (!kttsdLoaded) {
        m_jobMgrWidget = new KttsJobMgr (this);
        if (m_jobMgrWidget) {
            connect (m_jobMgrWidget, SIGNAL (configChanged()), this, SLOT (configChanged()));
            // Add the Job Manager part as a new tab.
            mainTab->addTab (m_jobMgrWidget, i18n ("Jobs"));
            kttsdLoaded = true;
        } else
            kDebug() << "KCMKttsMgr::jovieStarted: Could not create kttsjobmgr part.";
    }
    // Check/Uncheck the Enable KTTSD check box.
    if (kttsdLoaded) {
        enableJovieCheckBox->setChecked (true);
        m_kspeech = new OrgKdeKSpeechInterface (QLatin1String( "org.kde.jovie" ), QLatin1String( "/KSpeech" ), QDBusConnection::sessionBus());
        m_kspeech->setParent (this);
        m_kspeech->setApplicationName (QLatin1String( "KCMKttsMgr" ));
        m_kspeech->setIsSystemManager (true);
        // Connect KTTSD DBUS signals to our slots.
        connect (m_kspeech, SIGNAL (kttsdStarted()),
                 this, SLOT (jovieStarted()));
        connect (m_kspeech, SIGNAL (kttsdExiting()),
                 this, SLOT (jovieExiting()));
        connect (QDBusConnection::sessionBus().interface(), SIGNAL (serviceUnregistered(QString)),
                 this, SLOT (slotServiceUnregistered(QString)));
        connect (QDBusConnection::sessionBus().interface(), SIGNAL (serviceOwnerChanged(QString,QString,QString)),
                 this, SLOT (slotServiceOwnerChanged(QString,QString,QString)));

        kttsdVersion->setText (i18n ("Jovie Version: %1", m_kspeech->version()));

    } else {
        enableJovieCheckBox->setChecked (false);
        delete m_kspeech;
        m_kspeech = 0;
    }
}

void KCMKttsMgr::slotServiceUnregistered (const QString &service)
{
    if (service == QLatin1String ("org.kde.jovie")) {
        jovieExiting();
    }

}

void KCMKttsMgr::slotServiceOwnerChanged (const QString &service, const QString &, const QString &newOwner)
{
    if (service == QLatin1String ("org.kde.jovie") && newOwner.isEmpty()) {
        jovieExiting();
    }
}

/**
* This slot is called whenever KTTSD is about to exit.
*/
void KCMKttsMgr::jovieExiting()
{
    // kDebug() << "KCMKttsMgr::kttsdExiting: Running";
    if (m_jobMgrWidget) {
        mainTab->removeTab (wpJobs);
        delete m_jobMgrWidget;
        m_jobMgrWidget = 0;
    }

    enableJovieCheckBox->setChecked (false);
    disconnect (QDBusConnection::sessionBus().interface(), 0, this, 0);
    delete m_kspeech;
    m_kspeech = 0;
    kttsdVersion->setText (i18n ("Jovie not running"));
}

/**
* User has requested display of talker configuration dialog.
*/
void KCMKttsMgr::slotConfigureTalkerButton_clicked()
{
    // Get highlighted Talker from ListView.
    QModelIndex modelIndex = talkersView->currentIndex();

    if (!modelIndex.isValid())
        return;

    TalkerCode tc = m_talkerListModel.getRow(modelIndex.row());

    QPointer<AddTalker> dlg = new AddTalker (this);
    dlg->setTalkerCode(tc);
    if (dlg->exec() == QDialog::Accepted) {
        TalkerCode code = dlg->getTalkerCode();

        // Change the existing talker code to this new code.
        m_talkerListModel.updateRow(modelIndex.row(), code);

        configChanged();
    }
}

void KCMKttsMgr::slotConfigureFilterButton_clicked()
{
    configureFilterItem();
}

/**
 * User has requested display of filter configuration dialog.
 */
void KCMKttsMgr::configureFilterItem()
{
    // Get highlighted plugin from Filter ListView and load into memory.
    QTreeView* lView;
    FilterListModel* model;
    lView = filtersView;
    model = &m_filterListModel;
    QModelIndex modelIndex = lView->currentIndex();
    if (!modelIndex.isValid()) return;
    FilterItem fi = model->getRow (modelIndex.row());
    QString filterID = fi.id;
    QString filterPlugInName = fi.plugInName;
    QString desktopEntryName = fi.desktopEntryName;
    if (desktopEntryName.isEmpty()) return;
    m_loadedFilterPlugIn = loadFilterPlugin (desktopEntryName);
    if (!m_loadedFilterPlugIn) return;
    // kDebug() << "KCMKttsMgr::slot_configureFilter: plugin for " << filterPlugInName << " loaded successfully.";

    // Tell plugin to load its configuration.
    // kDebug() << "KCMKttsMgr::slot_configureFilter: about to call plugin load() method with Filter ID = " << filterID;
    m_loadedFilterPlugIn->load (m_config, QLatin1String ("Filter_") + filterID);

    // Display configuration dialog.
    configureFilter();

    // Did user Cancel?
    if (!m_loadedFilterPlugIn) {
        delete m_configDlg;
        m_configDlg = 0;
        return;
    }

    // Get user's name for the plugin.
    QString userFilterName = m_loadedFilterPlugIn->userPlugInName();

    // If user properly configured the plugin, save the configuration.
    if (!userFilterName.isEmpty()) {
        // Let plugin save its configuration.
        m_loadedFilterPlugIn->save (m_config, QLatin1String ("Filter_") + filterID);

        // Save configuration.
        KConfigGroup filterConfig (m_config, QLatin1String ("Filter_") + filterID);
        filterConfig.writeEntry ("DesktopEntryName", desktopEntryName);
        filterConfig.writeEntry ("UserFilterName", userFilterName);
        filterConfig.writeEntry ("Enabled", true);
        filterConfig.writeEntry ("MultiInstance", m_loadedFilterPlugIn->supportsMultiInstance());

        m_config->sync();

        // Update display.
        FilterItem fi;
        fi.id = filterID;
        fi.desktopEntryName = desktopEntryName;
        fi.userFilterName = userFilterName;
        fi.enabled = true;
        fi.multiInstance = m_loadedFilterPlugIn->supportsMultiInstance();
        model->updateRow (modelIndex.row(), fi);
        // Inform Control Center that configuration has changed.
        configChanged();
    }

    delete m_configDlg;
    m_configDlg = 0;
}

/**
* Display filter configuration dialog.  The plugin is assumed already loaded into
* memory referenced by m_loadedFilterPlugIn.
*/
void KCMKttsMgr::configureFilter()
{
    if (!m_loadedFilterPlugIn) return;
    m_configDlg = new KDialog (this);
    m_configDlg->setCaption (i18n ("Filter Configuration"));
    m_configDlg->setButtons (KDialog::Help | KDialog::Default | KDialog::Ok | KDialog::Cancel);
    m_configDlg->setDefaultButton (KDialog::Cancel);
    m_loadedFilterPlugIn->setMinimumSize (m_loadedFilterPlugIn->minimumSizeHint());
    m_loadedFilterPlugIn->show();
    m_configDlg->setMainWidget (m_loadedFilterPlugIn);
    m_configDlg->setHelp (QLatin1String( "configure-filter" ), QLatin1String( "jovie" ));
    m_configDlg->enableButtonOk (false);
    connect (m_loadedFilterPlugIn, SIGNAL (changed(bool)), this, SLOT (slotConfigFilterDlg_ConfigChanged()));
    connect (m_configDlg, SIGNAL (defaultClicked()), this, SLOT (slotConfigFilterDlg_DefaultClicked()));
    connect (m_configDlg, SIGNAL (cancelClicked()), this, SLOT (slotConfigFilterDlg_CancelClicked()));
    // Display the dialog.
    m_configDlg->exec();
}

/**
* Count number of configured Filters with the specified plugin name.
*/
int KCMKttsMgr::countFilterPlugins (const QString& filterPlugInName)
{
    int cnt = 0;
    for (int i = 0; i < m_filterListModel.rowCount(); ++i) {
        FilterItem fi = m_filterListModel.getRow (i);
        if (fi.plugInName == filterPlugInName) ++cnt;
    }
    return cnt;
}

void KCMKttsMgr::slotConfigTalkerDlg_ConfigChanged()
{
    //m_configDlg->enableButtonOk(!m_loadedTalkerPlugIn->getTalkerCode().isEmpty());
}

void KCMKttsMgr::slotConfigFilterDlg_ConfigChanged()
{
    m_configDlg->enableButtonOk (!m_loadedFilterPlugIn->userPlugInName().isEmpty());
}

void KCMKttsMgr::slotConfigTalkerDlg_DefaultClicked()
{
    //m_loadedTalkerPlugIn->defaults();
}

void KCMKttsMgr::slotConfigFilterDlg_DefaultClicked()
{
    m_loadedFilterPlugIn->defaults();
}

void KCMKttsMgr::slotConfigTalkerDlg_CancelClicked()
{
    //delete m_loadedTalkerPlugIn;
    //m_loadedTalkerPlugIn = 0;
}

void KCMKttsMgr::slotConfigFilterDlg_CancelClicked()
{
    delete m_loadedFilterPlugIn;
    m_loadedFilterPlugIn = 0;
}

/**
 * Uses KTrader to convert a translated Filter Plugin Name to DesktopEntryName.
 * @param name                   The translated plugin name.  From Name= line in .desktop file.
 * @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
 *                               QString() if not found.
 */
QString KCMKttsMgr::FilterNameToDesktopEntryName (const QString& name)
{
    if (name.isEmpty()) return QString();
    const KService::List  offers =  KServiceTypeTrader::self()->query (QLatin1String( "Jovie/FilterPlugin" ));
    for (int ndx = 0; ndx < offers.count(); ++ndx)
        if (offers[ndx]->name() == name)
            return offers[ndx]->desktopEntryName();
    return QString();
}

/**
 * Uses KTrader to convert a DesktopEntryName into a translated Filter Plugin Name.
 * @param desktopEntryName       The DesktopEntryName.
 * @return                       The translated Name of the plugin, from Name= line in .desktop file.
 */
QString KCMKttsMgr::FilterDesktopEntryNameToName (const QString& desktopEntryName)
{
    if (desktopEntryName.isEmpty()) return QString();
    KService::List offers = KServiceTypeTrader::self()->query (QLatin1String( "Jovie/FilterPlugin" ),
                            QString (QLatin1String( "DesktopEntryName == '%1'" )).arg (desktopEntryName));

    if (offers.count() == 1)
        return offers[0]->name();
    else
        return QString();
}


void KCMKttsMgr::slotFilterListView_clicked (const QModelIndex & index)
{
    if (!index.isValid()) return;
    if (index.column() != 0) return;
    if (index.row() < 0 || index.row() >= m_filterListModel.rowCount()) return;
    FilterItem fi = m_filterListModel.getRow (index.row());
    fi.enabled = !fi.enabled;
    m_filterListModel.updateRow (index.row(), fi);
    configChanged();
}
