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

#include "identity.h"

#include <QMetaProperty>
#include <QVariantMap>
#include <QString>

#include <KLocale>


#ifdef Q_OS_UNIX
#  include <sys/types.h>
#  include <pwd.h>
#  include <unistd.h>
#endif


INIT_SYNCABLE_OBJECT(Identity)
Identity::Identity(IdentityId id, QObject *parent)
    : SyncableObject(parent),
    _identityId(id)
{
    init();
    setToDefaults();
}


Identity::Identity(const Identity &other, QObject *parent)
    : SyncableObject(parent),
    _identityId(other.id()),
    _identityName(other.identityName()),
    _realName(other.realName()),
    _nicks(other.nicks()),
    _awayNick(other.awayNick()),
    _awayNickEnabled(other.awayNickEnabled()),
    _awayReason(other.awayReason()),
    _awayReasonEnabled(other.awayReasonEnabled()),
    _autoAwayEnabled(other.autoAwayEnabled()),
    _autoAwayTime(other.autoAwayTime()),
    _autoAwayReason(other.autoAwayReason()),
    _autoAwayReasonEnabled(other.autoAwayReasonEnabled()),
    _detachAwayEnabled(other.detachAwayEnabled()),
    _detachAwayReason(other.detachAwayReason()),
    _detachAwayReasonEnabled(other.detachAwayReasonEnabled()),
    _ident(other.ident()),
    _kickReason(other.kickReason()),
    _partReason(other.partReason()),
    _quitReason(other.quitReason())
{
    init();
}


void Identity::init()
{
    setObjectName(QString::number(id().toInt()));
    setAllowClientUpdates(true);
}


QString Identity::defaultNick()
{
    QString nick = QString("quassel%1").arg(qrand() & 0xff); // FIXME provide more sensible default nicks

#if   defined(Q_OS_UNIX)
    QString userName;
    struct passwd *pwd = getpwuid(getuid());
    if (pwd)
        userName = pwd->pw_name;
    if (!userName.isEmpty())
        nick = userName;

#endif

    // cleaning forbidden characters from nick
    QRegExp rx(QString("(^[\\d-]+|[^A-Za-z0-9\x5b-\x60\x7b-\x7d])"));
    nick.remove(rx);
    return nick;
}


QString Identity::defaultRealName()
{
    QString generalDefault = i18n("Kuassel User");

#if   defined(Q_OS_UNIX)
    QString realName;
    struct passwd *pwd = getpwuid(getuid());
    if (pwd)
        realName = QString::fromUtf8(pwd->pw_gecos);
    if (!realName.isEmpty())
        return realName;
    else
        return generalDefault;

#else
    return generalDefault;
#endif
}


void Identity::setToDefaults()
{
    setIdentityName(i18n("<empty>"));
    setRealName(defaultRealName());
    QStringList n = QStringList() << defaultNick();
    setNicks(n);
    setAwayNick("");
    setAwayNickEnabled(false);
    setAwayReason(i18n("Gone fishing."));
    setAwayReasonEnabled(true);
    setAutoAwayEnabled(false);
    setAutoAwayTime(10);
    setAutoAwayReason(i18n("Not here. No, really. not here!"));
    setAutoAwayReasonEnabled(false);
    setDetachAwayEnabled(false);
    setDetachAwayReason(i18n("All Quassel clients vanished from the face of the earth..."));
    setDetachAwayReasonEnabled(false);
    setIdent("quassel");
    setKickReason(i18n("Kindergarten is elsewhere!"));
    setPartReason(i18n("http://quassel-irc.org - Chat comfortably. Anywhere."));
    setQuitReason(i18n("http://quassel-irc.org - Chat comfortably. Anywhere."));
}


/*** setters ***/

void Identity::setId(IdentityId _id)
{
    _identityId = _id;
    SYNC(ARG(_id))
    emit idSet(_id);
    renameObject(QString::number(id().toInt()));
}


void Identity::setIdentityName(const QString &identityName)
{
    _identityName = identityName;
    SYNC(ARG(identityName))
}


void Identity::setRealName(const QString &realName)
{
    _realName = realName;
    SYNC(ARG(realName))
}


void Identity::setNicks(const QStringList &nicks)
{
    _nicks = nicks;
    SYNC(ARG(nicks))
    emit nicksSet(nicks);
}


void Identity::setAwayNick(const QString &nick)
{
    _awayNick = nick;
    SYNC(ARG(nick))
}


void Identity::setAwayReason(const QString &reason)
{
    _awayReason = reason;
    SYNC(ARG(reason))
}


void Identity::setAwayNickEnabled(bool enabled)
{
    _awayNickEnabled = enabled;
    SYNC(ARG(enabled))
}


void Identity::setAwayReasonEnabled(bool enabled)
{
    _awayReasonEnabled = enabled;
    SYNC(ARG(enabled))
}


void Identity::setAutoAwayEnabled(bool enabled)
{
    _autoAwayEnabled = enabled;
    SYNC(ARG(enabled))
}


void Identity::setAutoAwayTime(int time)
{
    _autoAwayTime = time;
    SYNC(ARG(time))
}


void Identity::setAutoAwayReason(const QString &reason)
{
    _autoAwayReason = reason;
    SYNC(ARG(reason))
}


void Identity::setAutoAwayReasonEnabled(bool enabled)
{
    _autoAwayReasonEnabled = enabled;
    SYNC(ARG(enabled))
}


void Identity::setDetachAwayEnabled(bool enabled)
{
    _detachAwayEnabled = enabled;
    SYNC(ARG(enabled))
}


void Identity::setDetachAwayReason(const QString &reason)
{
    _detachAwayReason = reason;
    SYNC(ARG(reason))
}


void Identity::setDetachAwayReasonEnabled(bool enabled)
{
    _detachAwayReasonEnabled = enabled;
    SYNC(ARG(enabled))
}


void Identity::setIdent(const QString &ident)
{
    _ident = ident;
    SYNC(ARG(ident))
}


void Identity::setKickReason(const QString &reason)
{
    _kickReason = reason;
    SYNC(ARG(reason))
}


void Identity::setPartReason(const QString &reason)
{
    _partReason = reason;
    SYNC(ARG(reason))
}


void Identity::setQuitReason(const QString &reason)
{
    _quitReason = reason;
    SYNC(ARG(reason))
}


/***  ***/

void Identity::copyFrom(const Identity &other)
{
    for (int idx = staticMetaObject.propertyOffset(); idx < staticMetaObject.propertyCount(); idx++) {
        QMetaProperty metaProp = staticMetaObject.property(idx);
        Q_ASSERT(metaProp.isValid());
        if (this->property(metaProp.name()) != other.property(metaProp.name())) {
            setProperty(metaProp.name(), other.property(metaProp.name()));
        }
    }
}


bool Identity::operator==(const Identity &other) const
{
    for (int idx = staticMetaObject.propertyOffset(); idx < staticMetaObject.propertyCount(); idx++) {
        QMetaProperty metaProp = staticMetaObject.property(idx);
        Q_ASSERT(metaProp.isValid());
        QVariant v1 = this->property(metaProp.name());
        QVariant v2 = other.property(metaProp.name()); // qDebug() << v1 << v2;
        // QVariant cannot compare custom types, so we need to check for this special case
        if (QString(v1.typeName()) == "IdentityId") {
            if (v1.value<IdentityId>() != v2.value<IdentityId>()) return false;
        }
        else {
            if (v1 != v2) return false;
        }
    }
    return true;
}


bool Identity::operator!=(const Identity &other) const
{
    return !(*this == other);
}


///////////////////////////////

QDataStream &operator<<(QDataStream &out, Identity id)
{
    out << id.toVariantMap();
    return out;
}


QDataStream &operator>>(QDataStream &in, Identity &id)
{
    QVariantMap i;
    in >> i;
    id.fromVariantMap(i);
    return in;
}


#ifdef HAVE_SSL
INIT_SYNCABLE_OBJECT(CertManager)
#endif // HAVE_SSL
