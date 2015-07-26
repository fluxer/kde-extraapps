/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "outputpage.h"
#include "ui_outputlocation.h"

#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/templaterenderer.h>

#include <KUrlRequester>
#include <KIntNumInput>
#include <KDebug>
#include <KFileDialog>

#include <QSignalMapper>
#include <QLabel>

namespace KDevelop {

struct OutputPagePrivate
{
    OutputPagePrivate(OutputPage* page_)
    : page(page_)
    , output(0)
    { }
    OutputPage* page;
    Ui::OutputLocationDialog* output;
    QSignalMapper urlChangedMapper;

    QHash<QString, KUrlRequester*> outputFiles;
    QHash<QString, KIntNumInput*> outputLines;
    QHash<QString, KIntNumInput*> outputColumns;
    QList<QLabel*> labels;

    QHash<QString, KUrl> defaultUrls;
    QHash<QString, KUrl> lowerCaseUrls;
    QStringList fileIdentifiers;

    void updateRanges(KIntNumInput* line, KIntNumInput* column, bool enable);
    void updateFileRange(const QString& field);
    void updateFileNames();
    void validate();
};

void OutputPagePrivate::updateRanges(KIntNumInput* line, KIntNumInput* column, bool enable)
{
    kDebug() << "Updating Ranges, file exists: " << enable;
    line->setEnabled(enable);
    column->setEnabled(enable);
}

void OutputPagePrivate::updateFileRange(const QString& field)
{
    if (!outputFiles.contains(field))
    {
        return;
    }

    QString url = outputFiles[field]->url().toLocalFile();
    QFileInfo info(url);

    updateRanges(outputLines[field], outputColumns[field], info.exists() && !info.isDir());

    validate();
}

void OutputPagePrivate::updateFileNames()
{
    bool lower = output->lowerFilenameCheckBox->isChecked();

    QHash<QString, KUrl> urls = lower ? lowerCaseUrls : defaultUrls;
    for (QHash<QString, KUrlRequester*>::const_iterator it = outputFiles.constBegin();
         it != outputFiles.constEnd(); ++it)
    {
        if (urls.contains(it.key()))
        {
            it.value()->setUrl(urls[it.key()]);
        }
    }

    //Save the setting for next time
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    codegenGroup.writeEntry( "LowerCaseFilenames", output->lowerFilenameCheckBox->isChecked() );

    validate();
}

void OutputPagePrivate::validate()
{
    QStringList invalidFiles;
    for(QHash< QString, KUrlRequester* >::const_iterator it = outputFiles.constBegin();
        it != outputFiles.constEnd(); ++it)
    {
        if (!it.value()->url().isValid()) {
            invalidFiles << it.key();
        } else if (it.value()->url().isLocalFile() && !QFileInfo(it.value()->url().upUrl().toLocalFile()).isWritable()) {
            invalidFiles << it.key();
        }
    }

    bool valid = invalidFiles.isEmpty();
    if (valid) {
        output->messageWidget->animatedHide();
    } else {
        qSort(invalidFiles);
        output->messageWidget->setMessageType(KMessageWidget::Error);
        output->messageWidget->setCloseButtonVisible(false);
        output->messageWidget->setText(i18np("Invalid output file: %2", "Invalid output files: %2", invalidFiles.count(), invalidFiles.join(", ")));
        output->messageWidget->animatedShow();
    }
    emit page->isValid(valid);
}

OutputPage::OutputPage(QWidget* parent)
: QWidget(parent)
, d(new OutputPagePrivate(this))
{
    d->output = new Ui::OutputLocationDialog;
    d->output->setupUi(this);
    d->output->messageWidget->setVisible(false);

    connect(&d->urlChangedMapper, SIGNAL(mapped(QString)),
            SLOT(updateFileRange(QString)));
    connect(d->output->lowerFilenameCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(updateFileNames()));
}

OutputPage::~OutputPage()
{
    delete d->output;
    delete d;
}

void OutputPage::prepareForm(const SourceFileTemplate& fileTemplate)
{
    // First clear any existing file configurations
    // This can happen when going back and forth between assistant pages
    d->fileIdentifiers.clear();
    d->defaultUrls.clear();
    d->lowerCaseUrls.clear();

    while (d->output->urlFormLayout->count() > 0)
    {
        d->output->urlFormLayout->takeAt(0);
    }
    while (d->output->positionFormLayout->count() > 0)
    {
        d->output->positionFormLayout->takeAt(0);
    }

    foreach (KUrlRequester* req, d->outputFiles)
    {
        d->urlChangedMapper.removeMappings(req);
    }

    qDeleteAll(d->outputFiles);
    qDeleteAll(d->outputLines);
    qDeleteAll(d->outputColumns);
    qDeleteAll(d->labels);

    d->outputFiles.clear();
    d->outputLines.clear();
    d->outputColumns.clear();
    d->labels.clear();

    foreach (const SourceFileTemplate::OutputFile& file, fileTemplate.outputFiles())
    {
        d->fileIdentifiers << file.identifier;

        QLabel* label = new QLabel(file.label, this);
        d->labels << label;
        KUrlRequester* requester = new KUrlRequester(this);
        requester->setMode( KFile::File | KFile::LocalOnly );
        requester->fileDialog()->setOperationMode( KFileDialog::Saving );

        d->urlChangedMapper.setMapping(requester, file.identifier);
        connect(requester, SIGNAL(textChanged(QString)), &d->urlChangedMapper, SLOT(map()));

        d->output->urlFormLayout->addRow(label, requester);
        d->outputFiles.insert(file.identifier, requester);

        label = new QLabel(file.label, this);
        d->labels << label;
        QHBoxLayout* layout = new QHBoxLayout(this);

        KIntNumInput* line = new KIntNumInput(this);
        line->setPrefix(i18n("Line: "));
        line->setValue(0);
        line->setMinimum(0);
        layout->addWidget(line);

        KIntNumInput* column = new KIntNumInput(this);
        column->setPrefix(i18n("Column: "));
        column->setValue(0);
        column->setMinimum(0);
        layout->addWidget(column);

        d->output->positionFormLayout->addRow(label, layout);
        d->outputLines.insert(file.identifier, line);
        d->outputColumns.insert(file.identifier, column);
    }
}

void OutputPage::loadFileTemplate(const SourceFileTemplate& fileTemplate,
                                   const KUrl& baseUrl,
                                   TemplateRenderer* renderer)
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup codegenGroup( config, "CodeGeneration" );
    bool lower = codegenGroup.readEntry( "LowerCaseFilenames", true );
    d->output->lowerFilenameCheckBox->setChecked(lower);

    foreach (const SourceFileTemplate::OutputFile& file, fileTemplate.outputFiles())
    {
        d->fileIdentifiers << file.identifier;

        KUrl url = baseUrl;
        url.addPath(renderer->render(file.outputName));
        d->defaultUrls.insert(file.identifier, url);

        url = baseUrl;
        url.addPath(renderer->render(file.outputName).toLower());
        d->lowerCaseUrls.insert(file.identifier, url);
    }

    d->updateFileNames();
}

QHash< QString, KUrl > OutputPage::fileUrls() const
{
    QHash<QString, KUrl> urls;
    for (QHash<QString, KUrlRequester*>::const_iterator it = d->outputFiles.constBegin(); it != d->outputFiles.constEnd(); ++it)
    {
        urls.insert(it.key(), it.value()->url());
    }
    return urls;
}

QHash< QString, SimpleCursor > OutputPage::filePositions() const
{
    QHash<QString, SimpleCursor> positions;
    foreach (const QString& identifier, d->fileIdentifiers)
    {
        positions.insert(identifier, SimpleCursor(d->outputLines[identifier]->value(), d->outputColumns[identifier]->value()));
    }
    return positions;
}

}

#include "moc_outputpage.cpp"
