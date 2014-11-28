/**
 * Copyright (C) 2004, 2007, 2009 Michael Pyne <mpyne@kde.org>
 * Copyright (C) 2003 Frerich Raabe <raabe@kde.org>
 * Copyright (C) 2014 Arnold Dumas <contact@arnolddumas.fr>
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

#include "filerenamer.h"

#include <algorithm>

#include <kdebug.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kvbox.h>

#include <QFile>
#include <QTimer>
#include <QCheckBox>
#include <QDir>
#include <QLabel>
#include <QSignalMapper>
#include <QPixmap>
#include <QFrame>
#include <QTreeWidget>

#include "tag.h"
#include "filerenameroptions.h"
#include "filehandle.h"
#include "exampleoptions.h"
#include "playlistitem.h"
#include "playlist.h" // processEvents()
#include "coverinfo.h"

class ConfirmationDialog : public KDialog
{
public:
    ConfirmationDialog(const QMap<QString, QString> &files,
                       QWidget *parent = 0, const char *name = 0)
        : KDialog(parent)
    {
        setObjectName( QLatin1String( name ) );
        setModal(true);
        setCaption(i18nc("warning about mass file rename", "Warning"));
        setButtons(Ok | Cancel);

        KVBox *vbox = new KVBox(this);
        setMainWidget(vbox);
        KVBox *hbox = new KVBox(vbox);

        QLabel *l = new QLabel(hbox);
        l->setPixmap(SmallIcon("dialog-warning", 32));

        l = new QLabel(i18n("You are about to rename the following files. "
                            "Are you sure you want to continue?"), hbox);
        hbox->setStretchFactor(l, 1);

        QTreeWidget *lv = new QTreeWidget(vbox);

        QStringList headers;
        headers << i18n("Original Name");
        headers << i18n("New Name");

        lv->setHeaderLabels(headers);
        lv->setRootIsDecorated(false);

        int lvHeight = 0;

        QMap<QString, QString>::ConstIterator it = files.constBegin();
        for(; it != files.constEnd(); ++it) {
            QTreeWidgetItem *item = new QTreeWidgetItem(lv);
            item->setText(0, it.key());

            if (it.key() != it.value()) {
                item->setText(1, it.value());
            }

            else {
                item->setText(1, i18n("No Change"));
            }

            lvHeight += lv->visualItemRect(item).height();
        }

        lvHeight += lv->horizontalScrollBar()->height() + lv->header()->height();
        lv->setMinimumHeight(qMin(lvHeight, 400));
        resize(qMin(width(), 500), qMin(minimumHeight(), 400));

        show();
    }
};

//
// Implementation of ConfigCategoryReader
//

ConfigCategoryReader::ConfigCategoryReader() : CategoryReaderInterface(),
    m_currentItem(0)
{
    KConfigGroup config(KGlobal::config(), "FileRenamer");

    QList<int> categoryOrder = config.readEntry("CategoryOrder", QList<int>());
    int categoryCount[NumTypes] = { 0 }; // Keep track of each category encountered.

    // Set a default:

    if(categoryOrder.isEmpty())
        categoryOrder << Artist << Album << Title << Track;

    QList<int>::ConstIterator catIt = categoryOrder.constBegin();
    for(; catIt != categoryOrder.constEnd(); ++catIt)
    {
        int catCount = categoryCount[*catIt]++;
        TagType category = static_cast<TagType>(*catIt);
        CategoryID catId(category, catCount);

        m_options[catId] = TagRenamerOptions(catId);
        m_categoryOrder << catId;
    }

    m_folderSeparators.fill(false, m_categoryOrder.count() - 1);

    QList<int> checkedSeparators = config.readEntry("CheckedDirSeparators", QList<int>());

    QList<int>::ConstIterator it = checkedSeparators.constBegin();
    for(; it != checkedSeparators.constEnd(); ++it) {
        if(*it < m_folderSeparators.count())
            m_folderSeparators[*it] = true;
    }

    m_musicFolder = config.readPathEntry("MusicFolder", "${HOME}/music");
    m_separator = config.readEntry("Separator", " - ");
}

QString ConfigCategoryReader::categoryValue(TagType type) const
{
    if(!m_currentItem)
        return QString();

    Tag *tag = m_currentItem->file().tag();

    switch(type) {
    case Track:
        return QString::number(tag->track());

    case Year:
        return QString::number(tag->year());

    case Title:
        return tag->title();

    case Artist:
        return tag->artist();

    case Album:
        return tag->album();

    case Genre:
        return tag->genre();

    default:
        return QString();
    }
}

QString ConfigCategoryReader::prefix(const CategoryID &category) const
{
    return m_options[category].prefix();
}

QString ConfigCategoryReader::suffix(const CategoryID &category) const
{
    return m_options[category].suffix();
}

TagRenamerOptions::EmptyActions ConfigCategoryReader::emptyAction(const CategoryID &category) const
{
    return m_options[category].emptyAction();
}

QString ConfigCategoryReader::emptyText(const CategoryID &category) const
{
    return m_options[category].emptyText();
}

QList<CategoryID> ConfigCategoryReader::categoryOrder() const
{
    return m_categoryOrder;
}

QString ConfigCategoryReader::separator() const
{
    return m_separator;
}

QString ConfigCategoryReader::musicFolder() const
{
    return m_musicFolder;
}

int ConfigCategoryReader::trackWidth(int categoryNum) const
{
    return m_options[CategoryID(Track, categoryNum)].trackWidth();
}

bool ConfigCategoryReader::hasFolderSeparator(int index) const
{
    if(index >= m_folderSeparators.count())
        return false;
    return m_folderSeparators[index];
}

bool ConfigCategoryReader::isDisabled(const CategoryID &category) const
{
    return m_options[category].disabled();
}

//
// Implementation of FileRenamerWidget
//

FileRenamerWidget::FileRenamerWidget(QWidget *parent) :
    QWidget(parent),
    CategoryReaderInterface(),
    m_ui(new Ui::FileRenamerBase),
    m_exampleFromFile(false)
{
    m_ui->setupUi(this);

    // This must be created before createTagRows() is called.

    m_exampleDialog = new ExampleOptionsDialog(this);

    createTagRows();
    loadConfig();

    // Add correct text to combo box.
    m_ui->m_category->clear();
    for(int i = StartTag; i < NumTypes; ++i) {
        QString category = TagRenamerOptions::tagTypeText(static_cast<TagType>(i));
        m_ui->m_category->addItem(category);
    }

    connect(m_exampleDialog, SIGNAL(signalShown()), SLOT(exampleDialogShown()));
    connect(m_exampleDialog, SIGNAL(signalHidden()), SLOT(exampleDialogHidden()));
    connect(m_exampleDialog, SIGNAL(dataChanged()), SLOT(dataSelected()));
    connect(m_exampleDialog, SIGNAL(fileChanged(QString)),
            this,            SLOT(fileSelected(QString)));

    exampleTextChanged();
}

void FileRenamerWidget::loadConfig()
{
    QList<int> checkedSeparators;
    KConfigGroup config(KGlobal::config(), "FileRenamer");

    for(int i = 0; i < m_rows.count(); ++i)
        m_rows[i].options = TagRenamerOptions(m_rows[i].category);

    checkedSeparators = config.readEntry("CheckedDirSeparators", QList<int>());

    foreach(int separator, checkedSeparators) {
        if(separator < m_folderSwitches.count())
            m_folderSwitches[separator]->setChecked(true);
    }

    QString path = config.readEntry("MusicFolder", "${HOME}/music");
    m_ui->m_musicFolder->setUrl(KUrl(path));
    m_ui->m_musicFolder->setMode(KFile::Directory |
                                 KFile::ExistingOnly |
                                 KFile::LocalOnly);

    m_ui->m_separator->setEditText(config.readEntry("Separator", " - "));
}

void FileRenamerWidget::saveConfig()
{
    KConfigGroup config(KGlobal::config(), "FileRenamer");
    QList<int> checkedSeparators;
    QList<int> categoryOrder;

    for(int i = 0; i < m_rows.count(); ++i) {
        int rowId = idOfPosition(i); // Write out in GUI order, not m_rows order
        m_rows[rowId].options.saveConfig(m_rows[rowId].category.categoryNumber);
        categoryOrder += m_rows[rowId].category.category;
    }

    for(int i = 0; i < m_folderSwitches.count(); ++i)
        if(m_folderSwitches[i]->isChecked() == true)
            checkedSeparators += i;

    config.writeEntry("CheckedDirSeparators", checkedSeparators);
    config.writeEntry("CategoryOrder", categoryOrder);
    config.writePathEntry("MusicFolder", m_ui->m_musicFolder->url().path());
    config.writeEntry("Separator", m_ui->m_separator->currentText());

    config.sync();
}

FileRenamerWidget::~FileRenamerWidget()
{
}

int FileRenamerWidget::addRowCategory(TagType category)
{
    static QIcon up   = SmallIcon("go-up");
    static QIcon down = SmallIcon("go-down");

    // Find number of categories already of this type.
    int categoryCount = 0;
    for(int i = 0; i < m_rows.count(); ++i)
        if(m_rows[i].category.category == category)
            ++categoryCount;

    Row row;

    row.category = CategoryID(category, categoryCount);
    row.position = m_rows.count();
    int id = row.position;

    QFrame *frame = new QFrame(m_mainFrame);
    QHBoxLayout *frameLayout = new QHBoxLayout(frame);
    frameLayout->setMargin(3);

    row.widget = frame;
    frame->setFrameShape(QFrame::Box);
    frame->setLineWidth(1);

    QBoxLayout *mainFrameLayout = static_cast<QBoxLayout *>(m_mainFrame->layout());
    mainFrameLayout->addWidget(frame, 1);

    QFrame *buttons = new QFrame(frame);
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttons);
    frameLayout->addWidget(buttons);
    buttons->setFrameStyle(QFrame::Plain | QFrame::Box);
    buttons->setLineWidth(1);

    row.upButton = new KPushButton(buttons);
    row.downButton = new KPushButton(buttons);

    row.upButton->setIcon(up);
    row.downButton->setIcon(down);
    row.upButton->setFlat(true);
    row.downButton->setFlat(true);

    upMapper->connect(row.upButton, SIGNAL(clicked()), SLOT(map()));
    upMapper->setMapping(row.upButton, id);
    downMapper->connect(row.downButton, SIGNAL(clicked()), SLOT(map()));
    downMapper->setMapping(row.downButton, id);

    buttonLayout->addWidget(row.upButton);
    buttonLayout->addWidget(row.downButton);

    QString labelText = QString("<b>%1</b>").arg(TagRenamerOptions::tagTypeText(category));
    QLabel *label = new QLabel(labelText, frame);
    frameLayout->addWidget(label, 1);
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout *optionLayout = new QVBoxLayout;
    frameLayout->addLayout(optionLayout);

    row.enableButton = new KPushButton(i18nc("remove music genre from file renamer", "Remove"), frame);
    optionLayout->addWidget(row.enableButton);
    toggleMapper->connect(row.enableButton, SIGNAL(clicked()), SLOT(map()));
    toggleMapper->setMapping(row.enableButton, id);

    row.optionsButton = new KPushButton(i18nc("file renamer genre options", "Options"), frame);
    optionLayout->addWidget(row.optionsButton);
    mapper->connect(row.optionsButton, SIGNAL(clicked()), SLOT(map()));
    mapper->setMapping(row.optionsButton, id);

    row.widget->show();
    m_rows.append(row);

    // Disable add button if there's too many rows.
    if(m_rows.count() == MAX_CATEGORIES)
        m_ui->m_insertCategory->setEnabled(false);

    return id;
}

void FileRenamerWidget::moveSignalMappings(int oldId, int newId)
{
    mapper->setMapping(m_rows[oldId].optionsButton, newId);
    downMapper->setMapping(m_rows[oldId].downButton, newId);
    upMapper->setMapping(m_rows[oldId].upButton, newId);
    toggleMapper->setMapping(m_rows[oldId].enableButton, newId);
}

bool FileRenamerWidget::removeRow(int id)
{
    if(id >= m_rows.count()) {
        kWarning() << "Trying to remove row, but " << id << " is out-of-range.\n";
        return false;
    }

    if(m_rows.count() == 1) {
        kError() << "Can't remove last row of File Renamer.\n";
        return false;
    }

    // Remove widget.  Don't delete it since it appears QSignalMapper may still need it.
    m_rows[id].widget->deleteLater();
    m_rows[id].widget = 0;
    m_rows[id].enableButton = 0;
    m_rows[id].upButton = 0;
    m_rows[id].optionsButton = 0;
    m_rows[id].downButton = 0;

    int checkboxPosition = 0; // Remove first checkbox.

    // If not the first row, remove the checkbox before it.
    if(m_rows[id].position > 0)
        checkboxPosition = m_rows[id].position - 1;

    // The checkbox is contained within a layout widget, so the layout
    // widget is the one the needs to die.
    delete m_folderSwitches[checkboxPosition]->parent();
    m_folderSwitches.erase(&m_folderSwitches[checkboxPosition]);

    // Go through all the rows and if they have the same category and a
    // higher categoryNumber, decrement the number.  Also update the
    // position identifier.
    for(int i = 0; i < m_rows.count(); ++i) {
        if(i == id)
            continue; // Don't mess with ourself.

        if((m_rows[id].category.category == m_rows[i].category.category) &&
           (m_rows[id].category.categoryNumber < m_rows[i].category.categoryNumber))
        {
            --m_rows[i].category.categoryNumber;
        }

        // Items are moving up.
        if(m_rows[id].position < m_rows[i].position)
            --m_rows[i].position;
    }

    // Every row after the one we delete will have a different identifier, since
    // the identifier is simply its index into m_rows.  So we need to re-do the
    // signal mappings for the affected rows.
    for(int i = id + 1; i < m_rows.count(); ++i)
        moveSignalMappings(i, i - 1);

    m_rows.erase(&m_rows[id]);

    // Make sure we update the buttons of affected rows.
    m_rows[idOfPosition(0)].upButton->setEnabled(false);
    m_rows[idOfPosition(m_rows.count() - 1)].downButton->setEnabled(false);

    // We can insert another row now, make sure GUI is updated to match.
    m_ui->m_insertCategory->setEnabled(true);

    QTimer::singleShot(0, this, SLOT(exampleTextChanged()));
    return true;
}

void FileRenamerWidget::addFolderSeparatorCheckbox()
{
    QWidget *temp = new QWidget(m_mainFrame);
    m_mainFrame->layout()->addWidget(temp);

    QHBoxLayout *l = new QHBoxLayout(temp);

    QCheckBox *cb = new QCheckBox(i18n("Insert folder separator"), temp);
    m_folderSwitches.append(cb);
    l->addWidget(cb, 0, Qt::AlignCenter);
    cb->setChecked(false);

    connect(cb, SIGNAL(toggled(bool)),
            SLOT(exampleTextChanged()));

    temp->show();
}

void FileRenamerWidget::createTagRows()
{
    KConfigGroup config(KGlobal::config(), "FileRenamer");
    QList<int> categoryOrder = config.readEntry("CategoryOrder", QList<int>());

    if(categoryOrder.isEmpty())
        categoryOrder << Artist << Album << Title << Track;

    // Setup arrays.
    m_rows.reserve(categoryOrder.count());
    m_folderSwitches.reserve(categoryOrder.count() - 1);

    mapper       = new QSignalMapper(this);
    mapper->setObjectName( QLatin1String("signal mapper" ));
    toggleMapper = new QSignalMapper(this);
    toggleMapper->setObjectName( QLatin1String("toggle mapper" ));
    upMapper     = new QSignalMapper(this);
    upMapper->setObjectName( QLatin1String("up button mapper" ));
    downMapper   = new QSignalMapper(this);
    downMapper->setObjectName( QLatin1String("down button mapper" ));

    connect(mapper,       SIGNAL(mapped(int)), SLOT(showCategoryOption(int)));
    connect(toggleMapper, SIGNAL(mapped(int)), SLOT(slotRemoveRow(int)));
    connect(upMapper,     SIGNAL(mapped(int)), SLOT(moveItemUp(int)));
    connect(downMapper,   SIGNAL(mapped(int)), SLOT(moveItemDown(int)));

    m_mainFrame = new QFrame(m_ui->m_mainView);
    m_ui->m_mainView->setWidget(m_mainFrame);
    m_ui->m_mainView->setWidgetResizable(true);

    QVBoxLayout *frameLayout = new QVBoxLayout(m_mainFrame);
    frameLayout->setMargin(10);
    frameLayout->setSpacing(5);

    // OK, the deal with the categoryOrder variable is that we need to create
    // the rows in the order that they were saved in (the order given by categoryOrder).
    // The signal mappers operate according to the row identifier.  To find the position of
    // a row given the identifier, use m_rows[id].position.  To find the id of a given
    // position, use idOfPosition(position).

    QList<int>::ConstIterator it = categoryOrder.constBegin();

    for(; it != categoryOrder.constEnd(); ++it) {
        if(*it < StartTag || *it >= NumTypes) {
            kError() << "Invalid category encountered in file renamer configuration.\n";
            continue;
        }

        if(m_rows.count() == MAX_CATEGORIES) {
            kError() << "Maximum number of File Renamer tags reached, bailing.\n";
            break;
        }

        TagType i = static_cast<TagType>(*it);

        addRowCategory(i);

        // Insert the directory separator checkbox if this isn't the last
        // item.

        QList<int>::ConstIterator dup(it);

        // Check for last item
        if(++dup != categoryOrder.constEnd())
            addFolderSeparatorCheckbox();
    }

    m_rows.first().upButton->setEnabled(false);
    m_rows.last().downButton->setEnabled(false);

    // If we have maximum number of categories already, don't let the user
    // add more.
    if(m_rows.count() >= MAX_CATEGORIES)
        m_ui->m_insertCategory->setEnabled(false);
}

void FileRenamerWidget::exampleTextChanged()
{
    // Just use .mp3 as an example
    if(m_exampleFromFile && (m_exampleFile.isEmpty() ||
                             !FileHandle(m_exampleFile).tag()->isValid()))
    {
        m_ui->m_exampleText->setText(i18n("No file selected, or selected file has no tags."));
        return;
    }

    m_ui->m_exampleText->setText(FileRenamer::fileName(*this) + ".mp3");
}

QString FileRenamerWidget::fileCategoryValue(TagType category) const
{
    FileHandle file(m_exampleFile);
    Tag *tag = file.tag();

    switch(category) {
    case Track:
        return QString::number(tag->track());

    case Year:
        return QString::number(tag->year());

    case Title:
        return tag->title();

    case Artist:
        return tag->artist();

    case Album:
        return tag->album();

    case Genre:
        return tag->genre();

    default:
        return QString();
    }
}

QString FileRenamerWidget::categoryValue(TagType category) const
{
    if(m_exampleFromFile)
        return fileCategoryValue(category);

    const ExampleOptions *example = m_exampleDialog->widget();

    switch (category) {
    case Track:
        return example->m_exampleTrack->text();

    case Year:
        return example->m_exampleYear->text();

    case Title:
        return example->m_exampleTitle->text();

    case Artist:
        return example->m_exampleArtist->text();

    case Album:
        return example->m_exampleAlbum->text();

    case Genre:
        return example->m_exampleGenre->text();

    default:
        return QString();
    }
}

QList<CategoryID> FileRenamerWidget::categoryOrder() const
{
    QList<CategoryID> list;

    // Iterate in GUI row order.
    for(int i = 0; i < m_rows.count(); ++i) {
        int rowId = idOfPosition(i);
        list += m_rows[rowId].category;
    }

    return list;
}

bool FileRenamerWidget::hasFolderSeparator(int index) const
{
    if(index >= m_folderSwitches.count())
        return false;
    return m_folderSwitches[index]->isChecked();
}

void FileRenamerWidget::moveItem(int id, MovementDirection direction)
{
    QWidget *l = m_rows[id].widget;
    int bottom = m_rows.count() - 1;
    int pos = m_rows[id].position;
    int newPos = (direction == MoveUp) ? pos - 1 : pos + 1;

    // Item we're moving can't go further down after this.

    if((pos == (bottom - 1) && direction == MoveDown) ||
       (pos == bottom && direction == MoveUp))
    {
        int idBottomRow = idOfPosition(bottom);
        int idAboveBottomRow = idOfPosition(bottom - 1);

        m_rows[idBottomRow].downButton->setEnabled(true);
        m_rows[idAboveBottomRow].downButton->setEnabled(false);
    }

    // We're moving the top item, do some button switching.

    if((pos == 0 && direction == MoveDown) || (pos == 1 && direction == MoveUp)) {
        int idTopItem = idOfPosition(0);
        int idBelowTopItem = idOfPosition(1);

        m_rows[idTopItem].upButton->setEnabled(true);
        m_rows[idBelowTopItem].upButton->setEnabled(false);
    }

    // This is the item we're swapping with.

    int idSwitchWith = idOfPosition(newPos);
    QWidget *w = m_rows[idSwitchWith].widget;

    // Update the table of widget rows.

    std::swap(m_rows[id].position, m_rows[idSwitchWith].position);

    // Move the item two spaces above/below its previous position.  It has to
    // be 2 spaces because of the checkbox.

    QBoxLayout *layout = dynamic_cast<QBoxLayout *>(m_mainFrame->layout());
    if ( !layout )
        return;

    layout->removeWidget(l);
    layout->insertWidget(2 * newPos, l);

    // Move the top item two spaces in the opposite direction, for a similar
    // reason.

    layout->removeWidget(w);
    layout->insertWidget(2 * pos, w);
    layout->invalidate();

    QTimer::singleShot(0, this, SLOT(exampleTextChanged()));
}

int FileRenamerWidget::idOfPosition(int position) const
{
    if(position >= m_rows.count()) {
        kError() << "Search for position " << position << " out-of-range.\n";
        return -1;
    }

    for(int i = 0; i < m_rows.count(); ++i)
        if(m_rows[i].position == position)
            return i;

    kError() << "Unable to find identifier for position " << position << endl;
    return -1;
}

int FileRenamerWidget::findIdentifier(const CategoryID &category) const
{
    for(int index = 0; index < m_rows.count(); ++index)
        if(m_rows[index].category == category)
            return index;

    kError() << "Unable to find match for category " <<
        TagRenamerOptions::tagTypeText(category.category) <<
        ", number " << category.categoryNumber << endl;

    return MAX_CATEGORIES;
}

void FileRenamerWidget::enableAllUpButtons()
{
    for(int i = 0; i < m_rows.count(); ++i)
        m_rows[i].upButton->setEnabled(true);
}

void FileRenamerWidget::enableAllDownButtons()
{
    for(int i = 0; i < m_rows.count(); ++i)
        m_rows[i].downButton->setEnabled(true);
}

void FileRenamerWidget::showCategoryOption(int id)
{
    TagOptionsDialog *dialog = new TagOptionsDialog(this, m_rows[id].options, m_rows[id].category.categoryNumber);

    if(dialog->exec() == QDialog::Accepted) {
        m_rows[id].options = dialog->options();
        exampleTextChanged();
    }

    delete dialog;
}

void FileRenamerWidget::moveItemUp(int id)
{
    moveItem(id, MoveUp);
}

void FileRenamerWidget::moveItemDown(int id)
{
    moveItem(id, MoveDown);
}

void FileRenamerWidget::toggleExampleDialog()
{
    m_exampleDialog->setHidden(!m_exampleDialog->isHidden());
}

void FileRenamerWidget::insertCategory()
{
    TagType category = static_cast<TagType>(m_ui->m_category->currentIndex());
    if(m_ui->m_category->currentIndex() < 0 || category >= NumTypes) {
        kError() << "Trying to add unknown category somehow.\n";
        return;
    }

    // We need to enable the down button of the current bottom row since it
    // can now move down.
    int idBottom = idOfPosition(m_rows.count() - 1);
    m_rows[idBottom].downButton->setEnabled(true);

    addFolderSeparatorCheckbox();

    // Identifier of new row.
    int id = addRowCategory(category);

    // Set its down button to be disabled.
    m_rows[id].downButton->setEnabled(false);

    m_mainFrame->layout()->invalidate();
    m_ui->m_mainView->update();

    // Now update according to the code in loadConfig().
    m_rows[id].options = TagRenamerOptions(m_rows[id].category);
    exampleTextChanged();
}

void FileRenamerWidget::exampleDialogShown()
{
    m_ui->m_showExample->setText(i18n("Hide Renamer Test Dialog"));
}

void FileRenamerWidget::exampleDialogHidden()
{
    m_ui->m_showExample->setText(i18n("Show Renamer Test Dialog"));
}

void FileRenamerWidget::fileSelected(const QString &file)
{
    m_exampleFromFile = true;
    m_exampleFile = file;
    exampleTextChanged();
}

void FileRenamerWidget::dataSelected()
{
    m_exampleFromFile = false;
    exampleTextChanged();
}

QString FileRenamerWidget::separator() const
{
    return m_ui->m_separator->currentText();
}

QString FileRenamerWidget::musicFolder() const
{
    return m_ui->m_musicFolder->url().path();
}

void FileRenamerWidget::slotRemoveRow(int id)
{
    // Remove the given identified row.
    if(!removeRow(id))
        kError() << "Unable to remove row " << id << endl;
}

//
// Implementation of FileRenamer
//

FileRenamer::FileRenamer()
{
}

void FileRenamer::rename(PlaylistItem *item)
{
    PlaylistItemList list;
    list.append(item);

    rename(list);
}

void FileRenamer::rename(const PlaylistItemList &items)
{
    ConfigCategoryReader reader;
    QStringList errorFiles;
    QMap<QString, QString> map;
    QMap<QString, PlaylistItem *> itemMap;

    for(PlaylistItemList::ConstIterator it = items.constBegin(); it != items.constEnd(); ++it) {
        reader.setPlaylistItem(*it);
        QString oldFile = (*it)->file().absFilePath();
        QString extension = (*it)->file().fileInfo().suffix();
        QString newFile = fileName(reader) + '.' + extension;

        if(oldFile != newFile) {
            map[oldFile] = newFile;
            itemMap[oldFile] = *it;
        }
    }

    if(itemMap.isEmpty() || ConfirmationDialog(map).exec() != QDialog::Accepted)
        return;

    KApplication::setOverrideCursor(Qt::waitCursor);
    for(QMap<QString, QString>::ConstIterator it = map.constBegin();
        it != map.constEnd(); ++it)
    {
        if(moveFile(it.key(), it.value())) {
            itemMap[it.key()]->setFile(it.value());
            itemMap[it.key()]->refresh();

            setFolderIcon(it.value(), itemMap[it.key()]);
        }
        else
            errorFiles << i18n("%1 to %2", it.key(), it.value());

        processEvents();
    }
    KApplication::restoreOverrideCursor();

    if(!errorFiles.isEmpty())
        KMessageBox::errorList(0, i18n("The following rename operations failed:\n"), errorFiles);
}

bool FileRenamer::moveFile(const QString &src, const QString &dest)
{
    kDebug() << "Moving file " << src << " to " << dest;

    if(src == dest)
        return false;

    // Escape URL.
    KUrl srcURL = KUrl(src);
    KUrl dstURL = KUrl(dest);

    // Clean it.
    srcURL.cleanPath();
    dstURL.cleanPath();

    // Make sure it is valid.
    if(!srcURL.isValid() || !dstURL.isValid())
        return false;

    // Get just the directory.
    KUrl dir = dstURL;
    dir.setFileName(QString());

    // Create the directory.
    if(!KStandardDirs::exists(dir.path()))
        if(!KStandardDirs::makeDir(dir.path())) {
            kError() << "Unable to create directory " << dir.path() << endl;
            return false;
        }

    // Move the file.
    KIO::Job *job = KIO::file_move(srcURL, dstURL);
    return KIO::NetAccess::synchronousRun(job, 0);
}

void FileRenamer::setFolderIcon(const KUrl &dst, const PlaylistItem *item)
{
    if(item->file().tag()->album().isEmpty() ||
       !item->file().coverInfo()->hasCover())
    {
        return;
    }

    KUrl dstURL = dst;
    dstURL.cleanPath();

    // Split path, and go through each path element.  If a path element has
    // the album information, set its folder icon.
    QStringList elements = dstURL.directory().split('/', QString::SkipEmptyParts);
    QString path;

    for(QStringList::ConstIterator it = elements.constBegin(); it != elements.constEnd(); ++it) {
        path.append('/' + (*it));

        kDebug() << "Checking path: " << path;
        if((*it).contains(item->file().tag()->album() ) &&
           !QFile::exists(path + "/.directory"))
        {
            // Seems to be a match, let's set the folder icon for the current
            // path.  First we should write out the file.

            QPixmap thumb = item->file().coverInfo()->pixmap(CoverInfo::Thumbnail);
            thumb.save(path + "/.juk-thumbnail.png", "PNG");

            KDesktopFile dirFile(path + "/.directory");
            KConfigGroup desktopGroup(dirFile.desktopGroup());

            if(!desktopGroup.hasKey("Icon")) {
                desktopGroup.writePathEntry("Icon", QString("%1/.juk-thumbnail.png").arg(path));
                dirFile.sync();
            }

            return;
        }
    }
}

/**
 * Returns iterator pointing to the last item enabled in the given list with
 * a non-empty value (or is required to be included).
 */
QList<CategoryID>::ConstIterator lastEnabledItem(const QList<CategoryID> &list,
                                                 const CategoryReaderInterface &interface)
{
    QList<CategoryID>::ConstIterator it = list.constBegin();
    QList<CategoryID>::ConstIterator last = list.constEnd();

    for(; it != list.constEnd(); ++it) {
        if(interface.isRequired(*it) || (!interface.isDisabled(*it) &&
              !interface.categoryValue((*it).category).isEmpty()))
        {
            last = it;
        }
    }

    return last;
}

QString FileRenamer::fileName(const CategoryReaderInterface &interface)
{
    const QList<CategoryID> categoryOrder = interface.categoryOrder();
    const QString separator = interface.separator();
    const QString folder = interface.musicFolder();
    QList<CategoryID>::ConstIterator lastEnabled;
    int i = 0;
    QStringList list;
    QChar dirSeparator (QDir::separator());

    // Use lastEnabled to properly handle folder separators.
    lastEnabled = lastEnabledItem(categoryOrder, interface);
    bool pastLast = false; // Toggles to true once we've passed lastEnabled.

    for(QList<CategoryID>::ConstIterator it = categoryOrder.constBegin();
            it != categoryOrder.constEnd();
            ++it, ++i)
    {
        if(it == lastEnabled)
            pastLast = true;

        if(interface.isDisabled(*it))
            continue;

        QString value = interface.value(*it);

        // The user can use the folder separator checkbox to add folders, so don't allow
        // slashes that slip in to accidentally create new folders.  Should we filter this
        // back out when showing it in the GUI?
        value.replace('/', "%2f");

        if(!pastLast && interface.hasFolderSeparator(i))
            value.append(dirSeparator);

        if(interface.isRequired(*it) || !value.isEmpty())
            list.append(value);
    }

    // Construct a single string representation, handling strings ending in
    // '/' specially

    QString result;

    for(QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); /* Empty */) {
        result += *it;

        ++it; // Manually advance iterator to check for end-of-list.

        // Add separator unless at a directory boundary
        if(it != list.constEnd() &&
           !(*it).startsWith(dirSeparator) && // Check beginning of next item.
           !result.endsWith(dirSeparator))
        {
            result += separator;
        }
    }

    return QString(folder + dirSeparator + result);
}

#include "filerenamer.moc"

// vim: set et sw=4 tw=0 sta:
