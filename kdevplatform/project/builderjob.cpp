/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "builderjob.h"

#include <kconfiggroup.h>
#include <KLocalizedString>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/interfaces/ibuildsystemmanager.h>

using namespace KDevelop;

struct SubJobData
{
    BuilderJob::BuildType type;
    KJob* job;
    ProjectBaseItem* item;
};
Q_DECLARE_TYPEINFO(SubJobData, Q_MOVABLE_TYPE);

namespace KDevelop
{
class BuilderJobPrivate
{
public:
    BuilderJobPrivate( BuilderJob* job )
        : q(job)
        , failOnFirstError(true)
    {
    }

    BuilderJob* q;

    void addJob( BuilderJob::BuildType, ProjectBaseItem* );
    bool failOnFirstError;

    QString buildTypeToString( BuilderJob::BuildType type ) const;

    bool hasJobForProject( BuilderJob::BuildType type, IProject* project ) const
    {
        foreach(const SubJobData& data, m_metadata) {
            if (data.type == type && data.item->project() == project) {
                return true;
            }
        }
        return false;
    }

    /**
     * a structure to keep metadata of all registered jobs
     */
    QVector<SubJobData> m_metadata;

    /**
     * get the subjob list and clear this composite job
     */
    QVector<SubJobData> takeJobList();
};
}

QString BuilderJobPrivate::buildTypeToString(BuilderJob::BuildType type) const
{
    switch( type ) {
        case BuilderJob::Build:
            return i18nc( "@info:status", "build" );
        case BuilderJob::Clean:
            return i18nc( "@info:status", "clean" );
        case BuilderJob::Configure:
            return i18nc( "@info:status", "configure" );
        case BuilderJob::Install:
            return i18nc( "@info:status", "install" );
        case BuilderJob::Prune:
            return i18nc( "@info:status", "prune" );
        default:
            return QString();
    }
}

void BuilderJobPrivate::addJob( BuilderJob::BuildType t, ProjectBaseItem* item )
{
    Q_ASSERT(item);
    kDebug() << "adding build job for item:" << item->text();
    Q_ASSERT(item->project());
    kDebug() << "project for item:" << item->project()->name();
    Q_ASSERT(item->project()->projectItem());
    kDebug() << "project item for the project:" << item->project()->projectItem()->text();
    if( !item->project()->buildSystemManager() )
    {
        kWarning() << "no buildsystem manager for:" << item->text() << item->project()->name();
        return;
    }
    kDebug() << "got build system manager";
    Q_ASSERT(item->project()->buildSystemManager()->builder());
    KJob* j = 0;
    switch( t )
    {
        case BuilderJob::Build:
            j = item->project()->buildSystemManager()->builder()->build( item );
            break;
        case BuilderJob::Clean:
            j = item->project()->buildSystemManager()->builder()->clean( item );
            break;
        case BuilderJob::Install:
            j = item->project()->buildSystemManager()->builder()->install( item );
            break;
        case BuilderJob::Prune:
            if (!hasJobForProject(t, item->project())) {
                j = item->project()->buildSystemManager()->builder()->prune( item->project() );
            }
            break;
        case BuilderJob::Configure:
            if (!hasJobForProject(t, item->project())) {
                j = item->project()->buildSystemManager()->builder()->configure( item->project() );
            }
            break;
        default:
            break;
    }
    if( j )
    {
        q->addCustomJob( t, j, item );
    }
}

BuilderJob::BuilderJob() 
    : d( new BuilderJobPrivate( this ) )
{
}

BuilderJob::~BuilderJob()
{
    delete d;
}

void BuilderJob::addItems( BuildType t, const QList<ProjectBaseItem*>& items )
{
    foreach( ProjectBaseItem* item, items )
    {
        d->addJob( t, item );
    }
}

void BuilderJob::addProjects( BuildType t, const QList<IProject*>& projects )
{
    foreach( IProject* project, projects )
    {
        d->addJob( t, project->projectItem() );
    }
}

void BuilderJob::addItem( BuildType t, ProjectBaseItem* item )
{
    d->addJob( t, item );
}

void BuilderJob::addCustomJob( BuilderJob::BuildType type, KJob* job, ProjectBaseItem* item )
{
    if( BuilderJob* builderJob = dynamic_cast<BuilderJob*>( job ) ) {
        // If a subjob is a builder job itself, re-own its job list to avoid having recursive composite jobs.
        QVector<SubJobData> subjobs = builderJob->d->takeJobList();
        builderJob->deleteLater();
        foreach( const SubJobData& subjob, subjobs ) {
            addSubjob( subjob.job );
        }
        d->m_metadata << subjobs;
    } else {
        addSubjob( job );

        SubJobData data;
        data.type = type;
        data.job = job;
        data.item = item;
        d->m_metadata << data;
    }
}

QVector< SubJobData > BuilderJobPrivate::takeJobList()
{
    QVector< SubJobData > ret = m_metadata;
    m_metadata.clear();
    q->clearSubjobs();
    q->setObjectName( QString() );
    return ret;
}

void BuilderJob::updateJobName()
{
    // Which items are mentioned in the set
    // Make it a list to preserve ordering; search overhead (n^2) isn't too big
    QList< ProjectBaseItem* > registeredItems;
    // Which build types are mentioned in the set
    // (Same rationale applies)
    QList< BuildType > buildTypes;
    // Whether there are jobs without any specific item
    bool hasNullItems = false;

    foreach( const SubJobData& subjob, d->m_metadata ) {
        if( subjob.item ) {
            if( !registeredItems.contains( subjob.item ) ) {
                registeredItems.append( subjob.item );
            }
            if( !buildTypes.contains( subjob.type ) ) {
                buildTypes.append( subjob.type );
            }
        } else {
            hasNullItems = true;
        }
    }

    QString itemNames;
    if( !hasNullItems ) {
        QStringList itemNamesList;
        foreach( ProjectBaseItem* item, registeredItems ) {
            itemNamesList << item->text();
        }
        itemNames = itemNamesList.join(", ");
    } else {
        itemNames = i18nc( "Unspecified set of build items (e. g. projects, targets)", "Various items" );
    }

    QString methodNames;
    QStringList methodNamesList;
    foreach( BuildType type, buildTypes ) {
        methodNamesList << d->buildTypeToString( type );
    }
    methodNames = methodNamesList.join( ", " );

    QString jobName = QString( "%1: %2" ).arg( itemNames ).arg( methodNames );
    setObjectName( jobName );
}

void BuilderJob::start()
{
    #if 0
    ///Running the same builder twice may result in serious problems, so kill jobs already running on the same project
    QList<QPointer<KJob> > jobs;
    foreach(KJob* job, ICore::self()->runController()->currentJobs()) {
        kDebug() << "running" << job;
        jobs << job;
    }
    
    for(QList< QPointer< KJob > >::iterator it = jobs.begin(); it != jobs.end(); ++it)
    {
        QPointer< KJob > job = *it;
        if(!job)
            continue;
        
        BuilderJob* bJob = dynamic_cast<BuilderJob*>(job.data());
        if( bJob && bJob != this )
        {
            kDebug() << "killing running builder job, due to new started build";

            //Copy the subjobs into QPointers first, as we never know what is deleted when
            QList<QPointer<KJob> > subJobs;
            foreach(KJob* subJob, bJob->subjobs())
                subJobs << subJob;
            
//             while(!subJobs.empty()) {
//                 if(subJobs.back() && subJobs.back()->capabilities() & KJob::Killable)
//                     subJobs.back()->kill(EmitResult);
//                 subJobs.pop_front();
//             }
            if(job && job->capabilities() & KJob::Killable)
                job->kill(EmitResult);
        }
    }
    #endif
    
    // Automatically save all documents before starting to build
    // might need an option to turn off at some point
    // Also should be moved into the builder and there try to find target(s) for the given item and then just save the documents of that target -> list??
    if( ICore::self()->activeSession()->config()->group("Project Manager").readEntry( "Save All Documents Before Building", true ) ) 
    {
        ICore::self()->documentController()->saveAllDocuments( IDocument::Silent );
    }

    if(hasSubjobs())
        ICore::self()->runController()->registerJob( subjobs().first() );
    else
        emitResult();
}

void BuilderJob::slotResult( KJob* job )
{
    //call parent implementation for default behaviour
    KCompositeJob::slotResult( job );
    if( ( !error() || !d->failOnFirstError ) && hasSubjobs() ) 
    {
        // start next build;
        ICore::self()->runController()->registerJob( subjobs().first() );
    } else 
    {
        emitResult();
    }
}

bool BuilderJob::addSubjob(KJob* job)
{
    // work-around for kdelibs bug: https://bugs.kde.org/show_bug.cgi?id=230692
    // (fixed in 4.11)
    // TODO: remove this method overwrite as soon as we depend on kdelibs 4.11
    bool success = KCompositeJob::addSubjob(job);
    // if job's parent != this => we're using kdelibs <= 4.10
    if (job->parent() != this) {
        // set the parent to 0, so the original parent won't delete this job
        // KCompositeJob takes care of deletion
        job->setParent(0);
    }
    return success;
}


void BuilderJob::setStopOnFail( bool stopOnFail )
{
    d->failOnFirstError = stopOnFail;
}

bool BuilderJob::stopOnFail() const
{
    return d->failOnFirstError;
}

#include "moc_builderjob.cpp"
