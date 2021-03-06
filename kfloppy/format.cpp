/*

    This file is part of the KFloppy program, part of the KDE project

    Copyright (C) 2002 Adriaan de Groot <groot@kde.org>
    Copyright (C) 2004, 2005 Nicolas GOUTTE <goutte@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "format.h"

#include <unistd.h>
#include <stdlib.h>

#include <QTimer>
#include <QRegExp>
#include <QProcess>

#include <klocale.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kstandarddirs.h>

KFAction::KFAction(QObject *parent) :
	QObject(parent)
{
	DEBUGSETUP;
}

KFAction::~KFAction()
{
	DEBUGSETUP;
	quit();
}

/* slot */ void KFAction::quit()
{
	DEBUGSETUP;
}

/* slot */ void KFAction::exec()
{
	DEBUGSETUP;
}

class KFActionQueue_p
{
public:
	QList<KFAction*> list;
} ;

KFActionQueue::KFActionQueue(QObject *parent) :
	KFAction(parent),
	d(new KFActionQueue_p)
{
	DEBUGSETUP;
}

KFActionQueue::~KFActionQueue()
{
	DEBUGSETUP;
	qDeleteAll(d->list);
	d->list.clear();
	delete d;
}

void KFActionQueue::queue(KFAction *p)
{
	DEBUGSETUP;

	d->list.append(p);
	DEBUGS(p->objectName());
}

/* virtual */ void KFActionQueue::exec()
{
	DEBUGSETUP;

	actionDone(0L,true);
}

/* slot */ void KFActionQueue::actionDone(KFAction *p,bool success)
{
	DEBUGSETUP;

	if (p)
	{
		if (!d->list.isEmpty() && d->list.first()==p)
		{
			d->list.removeFirst();
			delete p;
		}
		else
		{
			DEBUGS(  "Strange pointer received.");
			emit done(this,false);
			return;
		}
	}
	else
	{
		DEBUGS("Starting action queue.");
	}

	if (!success)
	{
		DEBUGS("Action failed.");
		emit done(this,false);
		return;
	}

	KFAction *next = d->list.isEmpty() ? 0 : d->list.first();
	if (!next)
	{
		emit done(this,true);
	}
	else
	{
		kDebug(KFAREA) << "Running action " << next->objectName() ;
		QObject::connect(next,SIGNAL(done(KFAction*,bool)),
			this,SLOT(actionDone(KFAction*,bool)));
		// Propagate signals
		QObject::connect(next,SIGNAL(status(QString,int)),
			this,SIGNAL(status(QString,int)));
		QTimer::singleShot(0,next,SLOT(exec()));
	}
}


// Here we have names of devices. The variable
// names are basically the linux device names,
// replace with whatever your OS needs instead.
//
//
#ifdef ANY_LINUX

const char * const fd0H1440[] = { "/dev/fd0u1440", "/dev/floppy/0u1440", "/dev/fd0h1440", "/dev/fd0H1440", 0L } ;
const char * const fd0D720[] = { "/dev/fd0u720", "/dev/floppy/0u720", "/dev/fd0D720", "/dev/fd0h720", 0L };
const char * const fd0h1200[] = { "/dev/fd0h1200", "/dev/floppy/0h1200", 0L };
const char * const fd0h360[] = { "/dev/fd0u360", "/dev/floppy/0u360", "/dev/fd0h360", "/dev/fd0d360", 0L };

const char * const fd1H1440[] = { "/dev/fd1u1440", "/dev/floppy/1u1440","/dev/fd1h1440", "/dev/fd1H1440", 0L } ;
const char * const fd1D720[] = { "/dev/fd1u720", "/dev/floppy/1u720", "/dev/fd1D720", "/dev/fd1h720", 0L };
const char * const fd1h1200[] = { "/dev/fd1h1200", "/dev/floppy/1h1200", 0L };
const char * const fd1h360[] = { "/dev/fd1u360", "/dev/floppy/1u360","/dev/fd1h360", "/dev/fd1d360", 0L };

const char * const fd0auto[] = { "/dev/fd0", 0L };
const char * const fd1auto[] = { "/dev/fd1", 0L };

#endif


#ifdef ANY_BSD
const char * const fd0[] = { "/dev/fd0", 0L } ;
const char * const fd1[] = { "/dev/fd1", 0L } ;
#endif

// Next we have a table of device names and characteristics.
// These are ordered according to 2*densityIndex+deviceIndex,
// ie. primary (0) 1440K (0) is first, then secondary (1) 1440K is
// second, down to secondary (1) 360k (4) in position 3*2+1=7.
//
//
// Note that the data originally contained in KFloppy was
// patently false, so most of this is fake. I guess no one ever
// formatted a 5.25" floppy.
//
// The flags field is unused in this implementation.
//
//
const fdinfo fdtable[] =
{
#ifdef ANY_LINUX
        // device  drv blks trk flg
	{ fd0H1440, 0, 1440, 80, 0 },
	{ fd1H1440, 1, 1440, 80, 0 },
	{ fd0D720,  0,  720, 80, 0 },
	{ fd1D720,  1,  720, 80, 0 },
	{ fd0h1200, 0, 1200, 80, 0 },
	{ fd1h1200, 1, 1200, 80, 0 },
	{ fd0h360,  0,  360, 40, 0 },
	{ fd1h360,  1,  360, 40, 0 },
	{ fd0auto,  0,    0, 80, 0 },
	{ fd1auto,  1,    0, 80, 0 },
#endif

#ifdef ANY_BSD
	// Instead of the number of tracks, which is
	// unneeded, we record the
	// number of F's printed during an fdformat
	{ fd0, 0, 1440, 40, 0 },
	{ fd1, 1, 1440, 40, 0 },
	{ fd0, 0,  720, 40, 0 },
	{ fd1, 1,  720, 40, 0 },
	{ fd0, 0, 1200, 40, 0},
	{ fd1, 1, 1200, 40, 0},
	{ fd0, 0,  360, 40, 0},
	{ fd1, 1,  360, 40, 0},
#endif
	{ 0L, 0, 0, 0, 0 }
} ;


FloppyAction::FloppyAction(QObject *p) :
	KFAction(p),
	deviceInfo(0L),
	theProcess(0L)
{
	DEBUGSETUP;
}

void FloppyAction::quit()
{
	DEBUGSETUP;
        delete theProcess;
        theProcess=0L;

	KFAction::quit();
}

bool FloppyAction::configureDevice( const QString& newDeviceName )
{
    deviceInfo = 0; // We have not any idea what the device is
    deviceName = newDeviceName;
    return true; // No problem!
}

bool FloppyAction::configureDevice(int drive,int density)
{
	DEBUGSETUP;
	const char *devicename = 0L;

	deviceInfo=0L;
	deviceName.clear();

	if ((drive<0) || (drive>1))
	{
		emit status(i18n("Unexpected drive number %1.", drive),-1);
		return false;
	}

	const fdinfo *deviceinfo = fdtable;
	for ( ; deviceinfo && (deviceinfo->devices) ; deviceinfo++)
	{
		if (deviceinfo->blocks != density)
			continue;
        }
	if (!deviceinfo)
	{
		emit status(i18n("Unexpected density number %1.", density),-1);
		return false;
	}

	deviceinfo = fdtable;
	for ( ; deviceinfo && (deviceinfo->devices) ; deviceinfo++)
	{
		if (deviceinfo->blocks != density)
			continue;
		if (deviceinfo->drive == drive)
			break;
	}

	if (!deviceinfo || !deviceinfo->devices)
	{
		emit status(i18n("Cannot find a device for drive %1 and density %2.",
			 drive, density),-1);
		return false;
	}

	for (const char* const* devices=deviceinfo->devices ;
		*devices ; devices++)
	{
		if (access(*devices,W_OK)>=0)
		{
			kDebug(KFAREA) << "Found device " << *devices ;
			devicename=*devices;
			break;
		}
	}

	if (!devicename)
	{
		const QString str = i18n(
			"Cannot access %1\nMake sure that the device exists and that "
			"you have write permission to it.",QLatin1String( deviceinfo->devices[0] ));
		emit status(str,-1);
		return false;
	}

	deviceName = QLatin1String( devicename );
	deviceInfo = deviceinfo;

	return true;
}

void FloppyAction::processDone(KProcess *p)
{
	DEBUGSETUP;

	if (p!=theProcess)
	{
		DEBUGS("  Strange process exited.");
		return;
	}

	if (p->state() == QProcess::NotRunning)
	{
	        if (p->exitStatus() == 0)
	        {
			emit status(QString(),100);	//krazy:exclude=nullstrassign for old broken gcc
			emit done(this,true);
		}
		else
		{
			emit status(i18n("The program %1 terminated with an error.", theProcessName),100);
			emit done(this,false);
		}
	}
	else
	{
		emit status(i18n("The program %1 terminated abnormally.", theProcessName),100);
		emit done(this,false);
	}
}

void FloppyAction::processStdOut(KProcess *, char *b, int l)
{
	Q_UNUSED(b);
	Q_UNUSED(l);
	kDebug(KFAREA) << "stdout:" << QString::fromLatin1(b,l) ;
}

void FloppyAction::processStdErr(KProcess *p, char *b, int l)
{
	processStdOut(p,b,l);
}

int FloppyAction::startProcess()
{
	DEBUGSETUP;

	connect(theProcess,SIGNAL(processExited(KProcess*)),
		this,SLOT(processDone(KProcess*)));
	connect(theProcess,SIGNAL(receivedStdout(KProcess*,char*,int)),
		this,SLOT(processStdOut(KProcess*,char*,int)));
	connect(theProcess,SIGNAL(receivedStderr(KProcess*,char*,int)),
		this,SLOT(processStdErr(KProcess*,char*,int)));

	 // We need the untranslated output of the tool
	theProcess->setEnv( QLatin1String( "LC_ALL" ), QLatin1String( "C" ) );
	theProcess->setOutputChannelMode(KProcess::MergedChannels);
	return theProcess->execute();
}


/* static */ QString FDFormat::fdformatName = QString();

FDFormat::FDFormat(QObject *p) :
	FloppyAction(p),
	doVerify(true)
{
	DEBUGSETUP;
	theProcessName = QString::fromLatin1("fdformat");
	setObjectName( QLatin1String("FDFormat" ));
}

/* static */ bool FDFormat::runtimeCheck()
{
	fdformatName = KStandardDirs::findRootExe(QLatin1String( "fdformat" ));
	return (!fdformatName.isEmpty());
}

bool FDFormat::configure(bool v)
{
	doVerify=v;
	return true;
}

/* virtual */ void FDFormat::exec()
{
	DEBUGSETUP;

	if ( !deviceInfo || deviceName.isEmpty() )
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (fdformatName.isEmpty())
	{
		emit status(i18n("Cannot find fdformat."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	theProcess = new KProcess;

	formatTrackCount=0;

	*theProcess << fdformatName ;

	// Common to Linux and BSD, others may differ
	if (!doVerify)
	{
		*theProcess << QLatin1String( "-n" );
	}

#ifdef ANY_BSD
	*theProcess
		<< QLatin1String( "-y" )
		<< QLatin1String( "-f" )
		<< QString::number(deviceInfo->blocks) ;
#elif defined(ANY_LINUX)
	// No Linux-specific flags
#endif

	// Common to Linux and BSD, others may differ
	*theProcess << deviceName;

	if (!startProcess())
	{
		emit status(i18n("Could not start fdformat."),-1);
		emit done(this,false);
	}

	// Now depend on fdformat running and producing output.
}

// Parse some output from the fdformat process. Lots of
// #ifdefs here to account for variations in the basic
// fdformat. Uses gotos to branch to whatever error message we
// need, since the messages can be standardized across OSsen.
//
//
void FDFormat::processStdOut(KProcess *, char *b, int l)
{
	DEBUGSETUP;
	QString s;

#ifdef ANY_BSD
	if (b[0]=='F')
	{
		formatTrackCount++;
		emit status(QString(),	//krazy:exclude=nullstrassign for old broken gcc
			formatTrackCount * 100 / deviceInfo->tracks);
	}
	else if (b[0]=='E')
	{
		emit status(i18n("Error formatting track %1.", formatTrackCount),-1);
	}
	else
	{
		s = QString::fromLatin1(b,l);
		if (s.contains(QLatin1String( "ioctl(FD_FORM)" )))
		{
                    emit status (i18n(
                            "Cannot access floppy or floppy drive.\n"
                            "Please insert a floppy and make sure that you "
                            "have selected a valid floppy drive."),-1);
                    return;
		}
		if (s.indexOf(QLatin1String( "/dev/" ))>=0)
		{
			emit status(s,-1);
			return;
		}
		DEBUGS(s);
	}
#elif defined(ANY_LINUX)
	s = QString::fromLatin1(b,l);
	DEBUGS(s);
        QRegExp regexp( QLatin1String( "([0-9]+)" ) );
        if ( s.startsWith( QLatin1String( "bad data at cyl" ) ) || s.contains( QLatin1String( "Problem reading cylinder" ) ) )
        {
            if ( regexp.indexIn( s ) > -1 )
            {
                const int track = regexp.cap(1).toInt();
                emit status(i18n("Low-level formatting error at track %1.", track), -1);
            }
            else
            {
                // This error should not happen
                emit status(i18n("Low-level formatting error: %1", s), -1);
            }
            return;
        }
	else if (s.contains(QLatin1String( "ioctl(FDFMTBEG)" )))
	{
            emit status (i18n(
                    "Cannot access floppy or floppy drive.\n"
                    "Please insert a floppy and make sure that you "
                    "have selected a valid floppy drive."),-1);
            return;
	}
        else if (s.contains(QLatin1String( "busy" ))) // "Device or resource busy"
        {
            emit status(i18n("Device busy.\nPerhaps you need to unmount the floppy first."),-1);
            return;
        }
        // Be careful to leave "iotcl" as last before checking numbers
        else if (s.contains(QLatin1String( "ioctl" )))
        {
            emit status(i18n("Low-level format error: %1", s),-1);
            return;
        }
        // Check for numbers at last (as /dev/fd0u1440 has numbers too)
        else if ( regexp.indexIn(s) > -1 )
        {
            // Normal track number (formatting or verifying)
            const int p = regexp.cap(1).toInt();
            if ((p>=0) && (p<deviceInfo->tracks))
            {
                    emit status(QString(),	//krazy:exclude=nullstrassign for old broken gcc
                            p * 100 / deviceInfo->tracks);
            }
        }
#endif
	return;
}


/* static */ QString DDZeroOut::m_ddName = QString();

DDZeroOut::DDZeroOut(QObject *p) :
    FloppyAction(p)
{
    kDebug(KFAREA);
    theProcessName = QString::fromLatin1("dd");
    setObjectName( QLatin1String("DD" ));
}

/* static */ bool DDZeroOut::runtimeCheck()
{
    m_ddName = KStandardDirs::findRootExe(QLatin1String( "dd" ));
    return (!m_ddName.isEmpty());
}

/* virtual */ void DDZeroOut::exec()
{
    kDebug(KFAREA);

    if ( deviceName.isEmpty() )
    {
        emit status( i18n("Internal error: device not correctly defined."), -1 );
        emit done( this, false );
        return;
    }

    if ( m_ddName.isEmpty() )
    {
        emit status( i18n("Cannot find dd."), -1 );
        emit done( this, false );
        return;
    }

    delete theProcess;
    theProcess = new KProcess;

    *theProcess << m_ddName ;

    *theProcess << QLatin1String( "if=/dev/zero" ) ;
    *theProcess << QLatin1String( "of=" )+deviceName;

    if ( !startProcess() )
    {
            emit status( i18n("Could not start dd."), -1 );
            emit done( this, false );
    }

}

void DDZeroOut::processDone(KProcess *p)
{
    kDebug(KFAREA);

    if (p!=theProcess)
    {
            kDebug(KFAREA) << "Strange process exited." ;
            return;
    }

    /**
     * As we do not give a number of blocks to dd(1), it will stop
     * with the error "No space left on device"
     *
     * ### TODO: really check if the exit is not on an other error and then abort the formatting
     */
    emit status(QString(),100);	//krazy:exclude=nullstrassign for old broken gcc
    emit done(this,true);
}


/* static */ QString FATFilesystem::newfs_fat = QString();

FATFilesystem::FATFilesystem(QObject *parent) :
	FloppyAction(parent)
{
	DEBUGSETUP;
	runtimeCheck();
	theProcessName=newfs_fat;
	setObjectName( QLatin1String("FATFilesystem" ));
}

/* static */ bool FATFilesystem::runtimeCheck()
{
	DEBUGSETUP;

#ifdef ANY_BSD
	newfs_fat = KStandardDirs::findRootExe(QLatin1String( "newfs_msdos" ));
#elif defined(ANY_LINUX)
	newfs_fat = KStandardDirs::findRootExe(QLatin1String( "mkdosfs" ));
#else
	return false;
#endif

	return !newfs_fat.isEmpty();
}

bool FATFilesystem::configure(bool v,bool l,const QString &lbl)
{
	doVerify=v;
	doLabel=l;
	if (l)
		label=lbl.simplified();
	else
		label.clear();

	return true;
}

void FATFilesystem::exec()
{
	DEBUGSETUP;

	if (
#ifdef ANY_BSD // BSD needs the deviceInfo for the block count
            !deviceInfo ||
#endif
            deviceName.isEmpty())
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (newfs_fat.isEmpty())
	{
		emit status(i18n("Cannot find a program to create FAT filesystems."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	KProcess *p = theProcess = new KProcess;

	*p << newfs_fat;
#ifdef ANY_BSD
	*p << QLatin1String( "-f" ) << QString::number(deviceInfo->blocks);
	if (doLabel)
	{
		*p << QLatin1String( "-L" ) << label ;
	}
#else
#ifdef ANY_LINUX
	if (doLabel)
	{
		*p << QLatin1String( "-n" ) << label ;
	}
	if (doVerify)
	{
		*p << QLatin1String( "-c" );
	}
#endif
#endif
	*p << deviceName ;

	if (!startProcess())
	{
		emit status(i18n("Cannot start FAT format program."),-1);
		emit done(this,false);
	}
}

void FATFilesystem::processStdOut(KProcess *, char *b, int l)
{
#ifdef ANY_BSD
    // ### TODO: do some checks
#elif defined(ANY_LINUX)
    QString s ( QString::fromLatin1( b, l ) );
    kDebug(KFAREA) << s ;
    if (s.contains(QLatin1String( "mounted file system" ))) // "/dev/fd0 contains a mounted file system
    {
        emit status(i18n("Floppy is mounted.\nYou need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains(QLatin1String( "busy" ))) // "Device or resource busy"
    {
        emit status(i18n("Device busy.\nPerhaps you need to unmount the floppy first."),-1);
        return;
    }
# if 0
    else if ( s.find( "mkdosfs" ) != -1 ) // DEBUG: get the program header and show it!
    {
        emit status( s, -1 );
        return;
    }
# endif
#endif
}


#ifdef ANY_BSD

/* static */ QString UFSFilesystem::newfs = QString();

UFSFilesystem::UFSFilesystem(QObject *parent) :
	FloppyAction(parent)
{
	DEBUGSETUP;
	runtimeCheck();
	theProcessName=newfs;
	setObjectName( QLatin1String("UFSFilesystem" ));
}

/* static */ bool UFSFilesystem::runtimeCheck()
{
	DEBUGSETUP;

	newfs = KStandardDirs::findRootExe(QLatin1String( "newfs" ));

	return !newfs.isEmpty();
}

void UFSFilesystem::exec()
{
	DEBUGSETUP;

	if ( deviceName.isEmpty() )
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (newfs.isEmpty())
	{
		emit status(i18nc("BSD", "Cannot find a program to create UFS filesystems."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	KProcess *p = theProcess = new KProcess;

	*p << newfs;

        // ### TODO: is it still needed? (FreeBSD 5.3's man page says: "For backward compatibility.")
        if ( deviceInfo )
           *p << "-T" << QString("fd%1").arg(deviceInfo->blocks);

        *p << deviceName;

	if (!startProcess())
	{
		emit status(i18nc("BSD", "Cannot start UFS format program."),-1);
		emit done(this,false);
	}
}
#endif


/* static */ QString Ext2Filesystem::newfs = QString();

Ext2Filesystem::Ext2Filesystem(QObject *parent) :
	FloppyAction(parent)
{
	DEBUGSETUP;
	runtimeCheck();
	theProcessName=QLatin1String( "mke2fs" );
	setObjectName( QLatin1String("Ext2Filesystem" ));
}

/* static */ bool Ext2Filesystem::runtimeCheck()
{
	DEBUGSETUP;

	newfs = KStandardDirs::findRootExe(QLatin1String( "mke2fs" ));

	return !newfs.isEmpty();
}

bool Ext2Filesystem::configure(bool v,bool l,const QString &lbl)
{
	doVerify=v;
	doLabel=l;
	if (l)
	{
		label=lbl.trimmed();
	}
	else
	{
		label.clear();
	}

	return true;
}

void Ext2Filesystem::exec()
{
	DEBUGSETUP;

	if (
#ifdef ANY_BSD // BSD needs the deviceInfo for the block count
            !deviceInfo ||
#endif
            deviceName.isEmpty() )
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (newfs.isEmpty())
	{
		emit status(i18n("Cannot find a program to create ext2 filesystems."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	KProcess *p = theProcess = new KProcess;

	*p << newfs;
	*p << "-q";
	if (doVerify) *p << QLatin1String( "-c" ) ;
	if (doLabel) *p << QLatin1String( "-L" ) << label ;

	*p << deviceName ;

	if (!startProcess())
	{
		emit status(i18n("Cannot start ext2 format program."),-1);
		emit done(this,false);
	}
}

void Ext2Filesystem::processStdOut(KProcess *, char *b, int l)
{
#ifdef ANY_BSD
    // ### TODO: do some checks
#elif defined(ANY_LINUX)
    QString s ( QString::fromLatin1( b, l ) );
    kDebug(KFAREA) << s ;
    if (s.contains(QLatin1String( "mounted" ))) // "/dev/fd0 is mounted; will not make a filesystem here!"
    {
        emit status(i18n("Floppy is mounted.\nYou need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains(QLatin1String( "busy" ))) // "Device or resource busy"
    {
        emit status(i18n("Device busy.\nPerhaps you need to unmount the floppy first."),-1);
        return;
    }
#endif
}



#ifdef ANY_LINUX
/* static */ QString MinixFilesystem::newfs = QString();

MinixFilesystem::MinixFilesystem(QObject *parent) :
	FloppyAction(parent)
{
	DEBUGSETUP;
	runtimeCheck();
	theProcessName=QLatin1String( "mkfs.minix" );
	setObjectName( QLatin1String("Minix2Filesystem" ));
}

/* static */ bool MinixFilesystem::runtimeCheck()
{
	DEBUGSETUP;

	newfs = KStandardDirs::findRootExe(QLatin1String( "mkfs.minix" ));

	return !newfs.isEmpty();
}

bool MinixFilesystem::configure(bool v,bool l,const QString &lbl)
{
	doVerify=v;
	doLabel=l;
	if (l)
	{
		label=lbl.trimmed();
	}
	else
	{
		label.clear();
	}

	return true;
}

void MinixFilesystem::exec()
{
	DEBUGSETUP;

	if ( deviceName.isEmpty() )
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (newfs.isEmpty())
	{
		emit status(i18n("Cannot find a program to create Minix filesystems."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	KProcess *p = theProcess = new KProcess;

	*p << newfs;

        // Labeling is not possible
	if (doVerify) *p << QLatin1String( "-c" ) ;

	*p << deviceName ;

	if (!startProcess())
	{
		emit status(i18n("Cannot start Minix format program."),-1);
		emit done(this,false);
	}
}

void MinixFilesystem::processStdOut(KProcess *, char *b, int l)
{
    QString s ( QString::fromLatin1( b, l ) );
    kDebug(KFAREA) << s ;
    if (s.contains(QLatin1String( "mounted" ))) // "mkfs.minix: /dev/fd0 is mounted; will not make a filesystem here!"
    {
        emit status(i18n("Floppy is mounted.\nYou need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains(QLatin1String( "busy" ))) // "Device or resource busy"
    {
        emit status(i18n("Device busy.\nPerhaps you need to unmount the floppy first."),-1);
        return;
    }
}

#endif

#include "moc_format.cpp"
