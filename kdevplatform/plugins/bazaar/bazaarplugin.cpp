/***************************************************************************
 *   Copyright 2013-2014 Maciej Poleski                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "bazaarplugin.h"

#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtGui/QMenu>

#include <KPluginFactory>
#include <KLocale>
#include <KAboutData>
#include <KStandardDirs>

#include <vcs/widgets/standardvcslocationwidget.h>
#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsstatusinfo.h>
#include <vcs/dvcs/ui/dvcsimportmetadatawidget.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>

#include "bazaarutils.h"
#include "bzrannotatejob.h"
#include "copyjob.h"
#include "diffjob.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevBazaarFactory, registerPlugin<BazaarPlugin>();)
K_EXPORT_PLUGIN(KDevBazaarFactory(
                    KAboutData("kdevbazaar",
                               "kdevbazaar",
                               ki18n("Bazaar"),
                               "1.0",
                               ki18n("A plugin to support Bazaar version control system"),
                               KAboutData::KAboutData::License_GPL)))

BazaarPlugin::BazaarPlugin(QObject* parent, const QVariantList& args) :
    IPlugin(KDevBazaarFactory::componentData(), parent),
    m_vcsPluginHelper(new KDevelop::VcsPluginHelper(this, this)), m_hasError(false)
{
    Q_UNUSED(args); // What is this?
    if (KStandardDirs::findExe("bzr").isEmpty()) {
        m_hasError = true;
        m_errorDescription = i18n("Bazaar is not installed (bzr executable not"
                                 " found)");
        return;
    }

    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBasicVersionControl)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IDistributedVersionControl)

    setObjectName("Bazaar");
}

BazaarPlugin::~BazaarPlugin()
{
}

QString BazaarPlugin::name() const
{
    return QString::fromUtf8("Bazaar");
}

VcsJob* BazaarPlugin::add(const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocations[0]), this);
    job->setType(VcsJob::Add);
    *job << "bzr" << "add";
    if(recursion == NonRecursive)
        *job << "--no-recurse";
    *job << localLocations;
    return job;
}

VcsJob* BazaarPlugin::annotate(const KUrl& localLocation, const VcsRevision& rev)
{
    VcsJob* job = new BzrAnnotateJob(BazaarUtils::workingCopy(localLocation), BazaarUtils::getRevisionSpec(rev), localLocation, this, KDevelop::OutputJob::Silent);
    return job;
}

VcsJob* BazaarPlugin::commit(const QString& message, const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    QDir dir = BazaarUtils::workingCopy(localLocations[0]);
    DVcsJob* job = new DVcsJob(dir, this);
    job->setType(VcsJob::Commit);

    *job << "bzr" << "commit" << BazaarUtils::handleRecursion(localLocations, recursion) << "-m" << message;
    return job;
}

VcsJob* BazaarPlugin::copy(const KUrl& localLocationSrc, const KUrl& localLocationDstn)
{
    return new CopyJob(localLocationSrc, localLocationDstn, this);
}

VcsImportMetadataWidget* BazaarPlugin::createImportMetadataWidget(QWidget* parent)
{
    return new DvcsImportMetadataWidget(parent);
}

VcsJob* BazaarPlugin::createWorkingCopy(const VcsLocation& sourceRepository, const KUrl& destinationDirectory, IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion);
    // What is the purpose of recursion parameter?
    DVcsJob* job = new DVcsJob(BazaarUtils::toQDir(sourceRepository.localUrl()), this);
    job->setType(VcsJob::Import);
    *job << "bzr" << "branch" << sourceRepository.localUrl().url() << destinationDirectory;
    return job;
}

VcsJob* BazaarPlugin::diff(const KUrl& fileOrDirectory, const VcsRevision& srcRevision, const VcsRevision& dstRevision, VcsDiff::Type, IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion);
    VcsJob* job = new DiffJob(BazaarUtils::workingCopy(fileOrDirectory), BazaarUtils::getRevisionSpecRange(srcRevision, dstRevision), fileOrDirectory, this);
    return job;
}

VcsJob* BazaarPlugin::init(const KUrl& localRepositoryRoot)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::toQDir(localRepositoryRoot), this);
    job->setType(VcsJob::Import);
    *job << "bzr" << "init";
    return job;
}

bool BazaarPlugin::isVersionControlled(const KUrl& localLocation)
{
    QDir workCopy = BazaarUtils::workingCopy(localLocation);
    DVcsJob* job = new DVcsJob(workCopy, this, OutputJob::Silent);
    job->setType(VcsJob::Unknown);
    job->setIgnoreError(true);
    *job << "bzr" << "ls" << "--from-root" << "-R" << "-V";
    job->exec();
    if (job->status() == VcsJob::JobSucceeded) {
        QList<QFileInfo> filesAndDirectoriesList;
        for (QString fod : job->output().split('\n')) {
            filesAndDirectoriesList.append(QFileInfo(workCopy.absolutePath() + QDir::separator() + fod));
        }
        QFileInfo fi(localLocation.toLocalFile());
        if (fi.isDir() || fi.isFile()) {
            QFileInfo file(localLocation.toLocalFile());
            return filesAndDirectoriesList.contains(file);
        }
    }
    return false;
}

VcsJob* BazaarPlugin::log(const KUrl& localLocation, const VcsRevision& rev, long unsigned int limit)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocation), this);
    job->setType(VcsJob::Log);
    *job << "bzr" << "log" << "--long" << "-v" << localLocation << BazaarUtils::getRevisionSpecRange(rev) << "-l" << QString::number(limit);
    connect(job, SIGNAL(readyForParsing(KDevelop::DVcsJob*)), this, SLOT(parseBzrLog(KDevelop::DVcsJob*)));
    return job;
}

VcsJob* BazaarPlugin::log(const KUrl& localLocation, const VcsRevision& rev, const VcsRevision& limit)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocation), this);
    job->setType(VcsJob::Log);
    *job << "bzr" << "log" << "--long" << "-v" << localLocation << BazaarUtils::getRevisionSpecRange(limit, rev);
    connect(job, SIGNAL(readyForParsing(KDevelop::DVcsJob*)), this, SLOT(parseBzrLog(KDevelop::DVcsJob*)));
    return job;
}

void BazaarPlugin::parseBzrLog(DVcsJob* job)
{
    QVariantList result;
    for (QString part : job->output().split("------------------------------------------------------------", QString::SkipEmptyParts)) {
        auto event = BazaarUtils::parseBzrLogPart(part);
        if (event.revision().revisionType() != VcsRevision::Invalid)
            result.append(QVariant::fromValue(event));
    }
    job->setResults(result);
}

VcsJob* BazaarPlugin::move(const KUrl& localLocationSrc, const KUrl& localLocationDst)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocationSrc), this);
    job->setType(VcsJob::VcsJob::Move);
    *job << "bzr" << "move" << localLocationSrc << localLocationDst;
    return job;
}

VcsJob* BazaarPlugin::pull(const VcsLocation& localOrRepoLocationSrc, const KUrl& localRepositoryLocation)
{
    // API describes hg pull which is git fetch equivalent
    // bzr has pull, but it succeds only if fast-forward is possible
    // in other cases bzr merge should be used instead (bzr pull would fail)
    // Information about repository must be provided at least once.
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localRepositoryLocation), this);
    job->setType(VcsJob::VcsJob::Pull);
    *job << "bzr" << "pull";
    if (!localOrRepoLocationSrc.localUrl().isEmpty()) {
        *job << localOrRepoLocationSrc.localUrl();
    }
    // localUrl always makes sense. Even on remote repositores which are handled
    // transparently.
    return job;
}

VcsJob* BazaarPlugin::push(const KUrl& localRepositoryLocation, const VcsLocation& localOrRepoLocationDst)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localRepositoryLocation), this);
    job->setType(VcsJob::VcsJob::Push);
    *job << "bzr" << "push" << localOrRepoLocationDst.localUrl();
    // localUrl always makes sense. Even on remote repositores which are handled
    // transparently.
    return job;
}

VcsJob* BazaarPlugin::remove(const KUrl::List& localLocations)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocations[0]), this);
    job->setType(VcsJob::VcsJob::Remove);
    *job << "bzr" << "remove" << localLocations;
    return job;
}

VcsJob* BazaarPlugin::repositoryLocation(const KUrl& localLocation)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocation), this);
    job->setType(VcsJob::VcsJob::Unknown);
    *job << "bzr" << "root" << localLocation;   // It is only to make sure
    connect(job, SIGNAL(readyForParsing(KDevelop::DVcsJob*)), this, SLOT(parseBzrRoot(KDevelop::DVcsJob*)));
    return job;
}

void BazaarPlugin::parseBzrRoot(DVcsJob* job)
{
    QString filename = job->dvcsCommand()[2];
    QString rootDirectory = job->output();
    QString localFilename = QFileInfo(QUrl(filename).toLocalFile()).absoluteFilePath();
    QString localRootDirectory = QFileInfo(rootDirectory).absolutePath();
    QString result = localFilename.mid(localFilename.indexOf(rootDirectory) + rootDirectory.length());
    job->setResults(QVariant::fromValue(result));
}

VcsJob* BazaarPlugin::resolve(const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    return add(localLocations, recursion);
    // How to provide "a conflict solving dialog to the user"?
    // In any case this plugin is unable to make any conflict.
}

VcsJob* BazaarPlugin::revert(const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocations[0]), this);
    job->setType(VcsJob::VcsJob::Revert);
    *job << "bzr" << "revert" << BazaarUtils::handleRecursion(localLocations, recursion);
    return job;
}

VcsJob* BazaarPlugin::status(const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion);
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocations[0]), this);
    job->setType(VcsJob::Status);
    *job << "bzr" << "status" << "--short" << "--no-pending" << "--no-classify" << localLocations;
    connect(job, SIGNAL(readyForParsing(KDevelop::DVcsJob*)), this, SLOT(parseBzrStatus(KDevelop::DVcsJob*)));
    return job;
}

void BazaarPlugin::parseBzrStatus(DVcsJob* job)
{
    QVariantList result;
    QSet<QString> filesWithStatus;
    QDir workingCopy = job->directory();
    for (QString line : job->output().split('\n')) {
        auto status = BazaarUtils::parseVcsStatusInfoLine(line);
        result.append(QVariant::fromValue(status));
        filesWithStatus.insert(BazaarUtils::concatenatePath(workingCopy, status.url()));
    }

    QStringList command = job->dvcsCommand();
    for (auto it = command.constBegin() + command.indexOf("--no-classify") + 1, itEnd = command.constEnd(); it != itEnd; ++it) {
        QString path = QFileInfo(*it).absoluteFilePath();
        if (!filesWithStatus.contains(path)) {
            filesWithStatus.insert(path);
            KDevelop::VcsStatusInfo status;
            status.setState(VcsStatusInfo::ItemUpToDate);
            status.setUrl(*it);
            result.append(QVariant::fromValue(status));
        }
    }

    job->setResults(result);
}

VcsJob* BazaarPlugin::update(const KUrl::List& localLocations, const VcsRevision& rev, IBasicVersionControl::RecursionMode recursion)
{
    // bzr update is stronger than API (it's effectively merge)
    // the best approximation is bzr pull
    DVcsJob* job = new DVcsJob(BazaarUtils::workingCopy(localLocations[0]), this);
    Q_UNUSED(recursion);
    // recursion and file locations are ignored - we can update only whole
    // working copy
    job->setType(VcsJob::VcsJob::Update);
    *job << "bzr" << "pull" << BazaarUtils::getRevisionSpec(rev);
    return job;
}

VcsLocationWidget* BazaarPlugin::vcsLocation(QWidget* parent) const
{
    return new KDevelop::StandardVcsLocationWidget(parent);
}

ContextMenuExtension BazaarPlugin::contextMenuExtension(Context* context)
{
    m_vcsPluginHelper->setupFromContext(context);
    KUrl::List const& ctxUrlList = m_vcsPluginHelper->contextUrlList();

    bool isWorkingDirectory = false;
    for (const KUrl & url : ctxUrlList) {
        if (BazaarUtils::isValidDirectory(url)) {
            isWorkingDirectory = true;
            break;
        }
    }

    if (!isWorkingDirectory) { // Not part of a repository
        return ContextMenuExtension();
    }

    QMenu* menu = m_vcsPluginHelper->commonActions();

    ContextMenuExtension menuExt;
    menuExt.addAction(ContextMenuExtension::VcsGroup, menu->menuAction());

    return menuExt;
}

bool BazaarPlugin::hasError() const
{
    return m_hasError;
}

QString BazaarPlugin::errorDescription() const
{
    return m_errorDescription;
}
