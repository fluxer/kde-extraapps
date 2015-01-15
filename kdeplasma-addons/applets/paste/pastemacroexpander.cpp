/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pastemacroexpander.h"

#include <QTextCodec>
#include <QFile>
#include <QProcess>
#include <QDate>
#include <QTime>

#include <KIO/NetAccess>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>
#include <KRandom>

class PasteMacroExpanderSingleton
{
    public:
        PasteMacroExpander instance;
};
K_GLOBAL_STATIC(PasteMacroExpanderSingleton, g_pasteMacroExpander)

PasteMacroExpander& PasteMacroExpander::instance()
 {
     return g_pasteMacroExpander->instance;
 }

PasteMacroExpander::PasteMacroExpander(QWidget *parent) : QObject(parent)
{
    m_macros["exec"] = QVariantList()
            << i18n("Execute Command And Get Output")
            << QVariant::fromValue(MacroParam(i18n("Command"), MacroParam::Url));
    m_macros["date"] = QVariantList()
            << i18n("Current Date");
    m_macros["time"] = QVariantList()
            << i18n("Current Time");
    m_macros["file"] = QVariantList()
            << i18n("Insert File Contents")
            << QVariant::fromValue(MacroParam(i18n("File"), MacroParam::Url));
    m_macros["password"] = QVariantList()
            << i18n("Random Password")
            << QVariant::fromValue(MacroParam(i18n("Character count"), MacroParam::Int))
            << QVariant::fromValue(MacroParam(i18n("Lowercase letters"), MacroParam::Boolean))
            << QVariant::fromValue(MacroParam(i18n("Uppercase letters"), MacroParam::Boolean))
            << QVariant::fromValue(MacroParam(i18n("Numbers"), MacroParam::Boolean))
            << QVariant::fromValue(MacroParam(i18n("Symbols"), MacroParam::Boolean));
}

QMap<QString, QVariantList> PasteMacroExpander::macros()
{
    return m_macros;
}

bool PasteMacroExpander::expandMacro(const QString &str, QStringList &ret)
{
    QString func;
    QString args;
    QString result;
    int n;

    if ((n = str.indexOf('(')) > 0) {
        func = str.left(n).trimmed();
        int m = str.lastIndexOf(')');
        args = str.mid(n + 1, m - n - 1);
    } else {
        func = str.trimmed();
    }
    //kDebug() << str << func << args;
    if (m_macros.keys().contains(func)) {
        QMetaObject::invokeMethod(this, func.toAscii(), Qt::DirectConnection,
                                  Q_RETURN_ARG(QString, result),
                                  Q_ARG(QString, args));
        ret += result;
        return true;
    }
    return false;
}

QString PasteMacroExpander::date(const QString& args)
{
    Q_UNUSED(args)
    return QDate::currentDate().toString();
}

QString PasteMacroExpander::time(const QString& args)
{
    Q_UNUSED(args)
    return QTime::currentTime().toString();
}

QString PasteMacroExpander::exec(const QString& args)
{
    QProcess process;

    process.start(args, QProcess::ReadOnly);
    process.waitForFinished();
    QByteArray ba = process.readAll();
    return QTextCodec::codecForLocale()->toUnicode(ba);
}

QString PasteMacroExpander::file(const QString& args)
{
    QString tmpFile;
    QString txt;
    QWidget *p = qobject_cast<QWidget*>(parent());

    if (KIO::NetAccess::download(args, tmpFile, p)) {
        QFile file(tmpFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            txt = QTextCodec::codecForLocale()->toUnicode(file.readAll());
        } else {
            KMessageBox::error(p, i18n("Could not open file: %1",tmpFile));
        }
        KIO::NetAccess::removeTempFile(tmpFile);
    } else {
        KMessageBox::error(p, KIO::NetAccess::lastErrorString());
    }
    return txt;
}

QString PasteMacroExpander::password(const QString& args)
{
    QStringList a = args.split(',', QString::SkipEmptyParts);
    static QStringList characterSets = QStringList()
            << "abcdefghijklmnopqrstuvwxyz"
            << "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            << "0123456789"
            << "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

    int charCount = 8;
    QString chars;
    QString result;

    if (a.count() > 0) {
        charCount = qMax(a[0].trimmed().toInt(), 8);
    }

    if (a.count() < 2) {
        chars = characterSets.join("");
    }

    if (a.count() > 1) {
        chars += (a[1].trimmed() == "true") ? characterSets[0] : "";
    }

    if (a.count() > 2) {
        chars += (a[2].trimmed() == "true") ? characterSets[1] : "";
    }

    if (a.count() > 3) {
        chars += (a[3].trimmed() == "true") ? characterSets[2] : "";
    }

    if (a.count() > 4) {
        chars += (a[4].trimmed() == "true") ? characterSets[3] : "";
    }

    const int setSize = chars.count();
    const int top = (RAND_MAX / setSize) * setSize;
    for (int i = 0; i < charCount; ++i) {
        // to prevent modulo bias, discard random numbers at the
        // 'top end' of INT_MAX
        int rand = -1;
        do {
            rand = KRandom::random();
        } while (rand >= top);

        result += chars[rand % setSize];
    }
    //kDebug() << result;
    return result;
}

#include "pastemacroexpander.moc"
