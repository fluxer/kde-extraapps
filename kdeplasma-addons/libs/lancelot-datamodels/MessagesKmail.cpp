/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "MessagesKmail.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

#include <KIcon>
#include <KRun>
#include <KToolInvocation>
#include <KStandardDirs>

#include "Logger.h"

#include "config-lancelot-datamodels.h"

#ifdef LANCELOT_THE_COMPILER_DOESNT_NEED_TO_PROCESS_THIS
// just in case messages:
I18N_NOOP("Unread messages");
I18N_NOOP("Unable to find Kontact");
I18N_NOOP("Start Akonadi server");
I18N_NOOP("Akonadi server is not running");
#endif


#warning "Pimlibs are not present"

    #define DummyModelClassName MessagesKmail
    #define DummyModelInit \
        setSelfTitle(i18n("Unread messages"));  \
        setSelfIcon(KIcon("kmail"));            \
        if (!addService("kontact") && !addService("kmail")) {   \
            add(i18n("Unable to find Kontact"), "",             \
                    KIcon("application-x-executable"),          \
                    QVariant("http://kontact.kde.org"));        \
        }

    #include "DummyModel_p.cpp"

    namespace Lancelot {
    namespace Models {

        class MessagesKmail::Private {
            void fetchEmailCollectionsDone(KJob * job)
            {
                Q_UNUSED(job);
            }
        };

        void MessagesKmail::updateLater()
        {
        }

        void MessagesKmail::update()
        {
        }

        QString MessagesKmail::selfShortTitle() const
        {
            return QString();
        }
    }
    }

    #undef DummyModelClassName
    #undef DummyModelInit


