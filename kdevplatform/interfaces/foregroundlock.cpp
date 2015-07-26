/*
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "foregroundlock.h"
#include <QMutex>
#include <QThread>
#include <QApplication>

using namespace KDevelop;

#define USE_PTHREAD_MUTEX

#if defined(USE_PTHREAD_MUTEX) && defined(Q_OS_LINUX)

#include <pthread.h>
#include <time.h>


class SimplePThreadMutex {
public:
    SimplePThreadMutex() {
	// this is not nessary because pthread_mutex_init() does the work, also it causes compile error on some systems
        //m_mutex = PTHREAD_MUTEX_INITIALIZER;
        int result = pthread_mutex_init(&m_mutex, 0);
        Q_ASSERT(result == 0);
        Q_UNUSED(result);
    }
    ~SimplePThreadMutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    void lockInline() {
        int result = pthread_mutex_lock(&m_mutex);
        Q_ASSERT(result == 0);
        Q_UNUSED(result);
    }
    void unlockInline() {
        int result = pthread_mutex_unlock(&m_mutex);
        Q_ASSERT(result == 0);
        Q_UNUSED(result);
    }
    bool tryLock(int interval) {
        if(interval == 0)
        {
            int result = pthread_mutex_trylock(&m_mutex);
            return result == 0;
        }else{
            timespec abs_time;
            clock_gettime(CLOCK_REALTIME, &abs_time);
            abs_time.tv_nsec += interval * 1000000;
            if(abs_time.tv_nsec >= 1000000000)
            {
                abs_time.tv_nsec -= 1000000000;
                abs_time.tv_sec += 1;
            }
            
            int result = pthread_mutex_timedlock(&m_mutex, &abs_time);
            return result == 0;
        }
    }
    
private:
    pthread_mutex_t m_mutex;
};

namespace {

SimplePThreadMutex internalMutex;
#else

namespace {

QMutex internalMutex;
#endif
QMutex tryLockMutex;
QMutex waitMutex;
QMutex finishMutex;
QWaitCondition condition;

volatile QThread* holderThread = 0;
volatile int recursion = 0;

void lockForegroundMutexInternal() {
    if(holderThread == QThread::currentThread())
    {
        // We already have the mutex
        ++recursion;
    }else{
        internalMutex.lockInline();
        Q_ASSERT(recursion == 0 && holderThread == 0);
        holderThread = QThread::currentThread();
        recursion = 1;
    }
}

bool tryLockForegroundMutexInternal(int interval = 0) {
    if(holderThread == QThread::currentThread())
    {
        // We already have the mutex
        ++recursion;
        return true;
    }else{
        if(internalMutex.tryLock(interval))
        {
            Q_ASSERT(recursion == 0 && holderThread == 0);
            holderThread = QThread::currentThread();
            recursion = 1;
            return true;
        }else{
            return false;
        }
    }
}

void unlockForegroundMutexInternal() {
    Q_ASSERT(holderThread == QThread::currentThread());
    Q_ASSERT(recursion > 0);
    recursion -= 1;
    if(recursion == 0)
    {
        holderThread = 0;
        internalMutex.unlockInline();
    }
}
}

ForegroundLock::ForegroundLock(bool lock) : m_locked(false)
{
    if(lock)
        relock();
}

void KDevelop::ForegroundLock::relock()
{
    Q_ASSERT(!m_locked);
    
    if(!QApplication::instance() || // Initialization isn't complete yet
        QThread::currentThread() == QApplication::instance()->thread() || // We're the main thread (deadlock might happen if we'd enter the trylock loop)
        holderThread == QThread::currentThread())  // We already have the foreground lock (deadlock might happen if we'd enter the trylock loop)
    {
        lockForegroundMutexInternal();
    }else{
        QMutexLocker lock(&tryLockMutex);
        
        while(!tryLockForegroundMutexInternal(10))
        {
            // In case an additional event-loop was started from within the foreground, we send
            // events to the foreground to temporarily release the lock.
            
            class ForegroundReleaser : public DoInForeground {
                public:
                virtual void doInternal() {
                    // By locking the mutex, we make sure that the requester is actually waiting for the condition
                    waitMutex.lockInline();
                    // Now we release the foreground lock
                    TemporarilyReleaseForegroundLock release;
                    // And signalize to the requester that we've released it
                    condition.wakeAll();
                    // Allow the requester to actually wake up, by unlocking m_waitMutex
                    waitMutex.unlockInline();
                    // Now wait until the requester is ready
                    QMutexLocker lock(&finishMutex);
                }
            };
            
            static ForegroundReleaser releaser;
            
            QMutexLocker lockWait(&waitMutex);
            QMutexLocker lockFinish(&finishMutex);
            
            QMetaObject::invokeMethod(&releaser, "doInternalSlot", Qt::QueuedConnection);
            // We limit the waiting time here, because sometimes it may happen that the foreground-lock is released,
            // and the foreground is waiting without an event-loop running. (For example through TemporarilyReleaseForegroundLock)
            condition.wait(&waitMutex, 30);
            
            if(tryLockForegroundMutexInternal())
            {
                //success
                break;
            }else{
                //Probably a third thread has creeped in and
                //got the foreground lock before us. Just try again.
            }
        }
    }
    m_locked = true;
    Q_ASSERT(holderThread == QThread::currentThread());
    Q_ASSERT(recursion > 0);
}

bool KDevelop::ForegroundLock::isLockedForThread()
{
    return QThread::currentThread() == holderThread;
}

bool KDevelop::ForegroundLock::tryLock()
{
    if(tryLockForegroundMutexInternal())
    {
        m_locked = true;
        return true;
    }
    return false;
}

void KDevelop::ForegroundLock::unlock()
{
    Q_ASSERT(m_locked);
    unlockForegroundMutexInternal();
    m_locked = false;
}

TemporarilyReleaseForegroundLock::TemporarilyReleaseForegroundLock()
{
    Q_ASSERT(holderThread == QThread::currentThread());
    
    m_recursion = 0;
    
    while(holderThread == QThread::currentThread())
    {
        unlockForegroundMutexInternal();
        ++m_recursion;
    }
}

TemporarilyReleaseForegroundLock::~TemporarilyReleaseForegroundLock()
{
    for(int a = 0; a < m_recursion; ++a)
        lockForegroundMutexInternal();
    Q_ASSERT(recursion == m_recursion && holderThread == QThread::currentThread());
}

KDevelop::ForegroundLock::~ForegroundLock()
{
    if(m_locked)
        unlock();
}

bool KDevelop::ForegroundLock::isLocked() const
{
    return m_locked;
}

namespace KDevelop {
    void DoInForeground::doIt() {
        if(QThread::currentThread() == QApplication::instance()->thread())
        {
            // We're already in the foreground, just call the handler code
            doInternal();
        }else{
            QMutexLocker lock(&m_mutex);
            QMetaObject::invokeMethod(this, "doInternalSlot", Qt::QueuedConnection);
            m_wait.wait(&m_mutex);
        }
    }

    DoInForeground::~DoInForeground() {
    }

    DoInForeground::DoInForeground() {
        moveToThread(QApplication::instance()->thread());
    }

    void DoInForeground::doInternalSlot()
    {
        VERIFY_FOREGROUND_LOCKED
        doInternal();
        QMutexLocker lock(&m_mutex);
        m_wait.wakeAll();
    }
}

// Important: The foreground lock has to be held by default, so lock it during static initialization
static struct StaticLock {
    StaticLock() {
        // Only lock, without unlocking, because otherwise important variables like
        // QThread::currentThread() might already be invalid during destruction.
        lockForegroundMutexInternal();
    }
} staticLock;
