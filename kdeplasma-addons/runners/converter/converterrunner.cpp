/*
 * Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
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

#include "converterrunner.h"
#include <QApplication>
#include <QClipboard>
#include <QSet>
#include <KIcon>
#include <KDebug>
#include <KToolInvocation>
#include <kunitconversion.h>

#define CONVERSION_CHAR QLatin1Char( '>' )

class StringParser
{
public:
    enum GetType
    {
        GetString = 1,
        GetDigit  = 2
    };

    StringParser(const QString &s) : m_index(0), m_s(s) {};
    ~StringParser() {};

    QString get(int type)
    {
        QChar current;
        QString result;

        passWhiteSpace();
        while (true) {
            current = next();
            if (current.isNull()) {
                break;
            }
            if (current.isSpace()) {
                break;
            }
            bool number = isNumber(current);
            if (type == GetDigit && !number) {
                break;
            }
            if (type == GetString && number) {
                break;
            }
            if (current == QLatin1Char(CONVERSION_CHAR)) {
                break;
            }
            ++m_index;
            result += current;
        }
        return result;
    }

    bool isNumber(const QChar &ch)
    {
        if (ch.isNumber()) {
            return true;
        }
        if (QString(QLatin1String( ".,-+" )).contains( ch )) {
            return true;
        }
        return false;
    }

    QString rest()
    {
        return m_s.mid(m_index).simplified();
    }

    void pass(const QStringList &strings)
    {
        passWhiteSpace();
        const QString temp = m_s.mid(m_index);

        foreach (const QString& s, strings) {
            if (temp.startsWith(s)) {
                m_index += s.length();
                return;
            }
        }
    }

private:
    void passWhiteSpace()
    {
        while (next().isSpace()) {
            ++m_index;
        }
    }

    QChar next()
    {
        if (m_index >= m_s.size()) {
            return QChar();
        }
        return m_s.at(m_index);
    }

    int m_index;
    QString m_s;
};

ConverterRunner::ConverterRunner(QObject* parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName(QLatin1String( "Converter" ));

    m_separators << QString( CONVERSION_CHAR );
    m_separators << i18nc("list of words that can used as amount of 'unit1' [in|to|as] 'unit2'",
                          "in;to;as").split(QLatin1Char( ';' ));

    //can not ignore commands: we have things like m4
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);

    QString description = i18n("Converts the value of :q: when :q: is made up of "
                               "\"value unit [>, to, as, in] unit\". You can use the "
                               "Unit converter applet to find all available units.");
    addSyntax(Plasma::RunnerSyntax(QLatin1String(":q:"), description));
}

ConverterRunner::~ConverterRunner()
{
}

void ConverterRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.size() < 2) {
        return;
    }

    StringParser cmd(term);
    QString unit1;
    QString value;
    QString unit2;

    unit1 = cmd.get(StringParser::GetString);
    value = cmd.get(StringParser::GetDigit);
    if (value.isEmpty()) {
        return;
    }
    if (unit1.isEmpty()) {
        unit1 = cmd.get(StringParser::GetString | StringParser::GetDigit);
        if (unit1.isEmpty()) {
            return;
        }
    }

    const QString s = cmd.get(StringParser::GetString);

    if (!s.isEmpty() && !m_separators.contains(s)) {
        unit1 += QLatin1Char(' ') + s;
    }
    if (s.isEmpty() || !m_separators.contains(s)) {
        cmd.pass(m_separators);
    }
    unit2 = cmd.rest();

    const double unit1value = value.toDouble();

    // qDebug() << Q_FUNC_INFO << unit1 << unit2 << unit1value;

    KTemperature temp(unit1value, unit1);
    if (temp.unitEnum() != KTemperature::Invalid) {
        KTemperature temp2(0.0, unit2);
        if (temp2.unitEnum() != KTemperature::Invalid) {
            const double result = temp.convertTo(temp2.unitEnum());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::InformationalMatch);
            match.setIcon(KIcon(QLatin1String("edit-copy")));
            match.setText(QString::fromLatin1("%1 (%2)").arg(QString::number(result), temp2.unit()));
            match.setData(result);
            context.addMatch(term, match);
            return;
        }
        for (int i = 0; i < KTemperature::UnitCount; i++) {
            KTemperature itemp(0.0, static_cast<KTemperature::KTempUnit>(i));
            const double result = temp.convertTo(itemp.unitEnum());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setIcon(KIcon(QLatin1String("edit-copy")));
            match.setText(QString::fromLatin1("%1 (%2)").arg(QString::number(result), itemp.unit()));
            match.setData(result);
            context.addMatch(term, match);
        }
        return;
    }

    KVelocity velo(unit1value, unit1);
    if (velo.unitEnum() != KVelocity::Invalid) {
        KVelocity velo2(0.0, unit2);
        if (velo2.unitEnum() != KVelocity::Invalid) {
            const double result = velo.convertTo(velo2.unitEnum());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::InformationalMatch);
            match.setIcon(KIcon(QLatin1String("edit-copy")));
            match.setText(QString::fromLatin1("%1 (%2)").arg(QString::number(result), velo2.unit()));
            match.setData(result);
            context.addMatch(term, match);
            return;
        }
        for (int i = 0; i < KVelocity::UnitCount; i++) {
            KVelocity ivelo(0.0, static_cast<KVelocity::KVeloUnit>(i));
            const double result = velo.convertTo(ivelo.unitEnum());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setIcon(KIcon(QLatin1String("edit-copy")));
            match.setText(QString::fromLatin1("%1 (%2)").arg(QString::number(result), ivelo.unit()));
            match.setData(result);
            context.addMatch(term, match);
        }
        return;
    }

    KPressure pres(unit1value, unit1);
    if (pres.unitEnum() != KPressure::Invalid) {
        KPressure pres2(0.0, unit2);
        if (pres2.unitEnum() != KPressure::Invalid) {
            const double result = pres.convertTo(pres2.unitEnum());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::InformationalMatch);
            match.setIcon(KIcon(QLatin1String("edit-copy")));
            match.setText(QString::fromLatin1("%1 (%2)").arg(QString::number(result), pres2.unit()));
            match.setData(result);
            context.addMatch(term, match);
            return;
        }
        for (int i = 0; i < KPressure::UnitCount; i++) {
            KPressure ipres(0.0, static_cast<KPressure::KPresUnit>(i));
            const double result = pres.convertTo(ipres.unitEnum());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setIcon(KIcon(QLatin1String("edit-copy")));
            match.setText(QString::fromLatin1("%1 (%2)").arg(QString::number(result), ipres.unit()));
            match.setData(result);
            context.addMatch(term, match);
        }
        return;
    }

    KLength leng(unit1value, unit1);
    if (leng.unitEnum() != KLength::Invalid) {
        KLength leng2(0.0, unit2);
        if (leng2.unitEnum() != KLength::Invalid) {
            const double result = leng.convertTo(leng2.unitEnum());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::InformationalMatch);
            match.setIcon(KIcon(QLatin1String("edit-copy")));
            match.setText(QString::fromLatin1("%1 (%2)").arg(QString::number(result), leng2.unit()));
            match.setData(result);
            context.addMatch(term, match);
            return;
        }
        for (int i = 0; i < KLength::UnitCount; i++) {
            KLength ileng(0.0, static_cast<KLength::KLengUnit>(i));
            const double result = leng.convertTo(ileng.unitEnum());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setIcon(KIcon(QLatin1String("edit-copy")));
            match.setText(QString::fromLatin1("%1 (%2)").arg(QString::number(result), ileng.unit()));
            match.setData(result);
            context.addMatch(term, match);
        }
    }
}

void ConverterRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
#warning TODO: notify the user that the data has been copied to the clipboard
    // other runners silently do the same (e.g. spellcheck), how would the user know what just happened?
    const QString data = match.data().toString();
    QApplication::clipboard()->setText(data);
}

#include "moc_converterrunner.cpp"
