/*
 * Copyright (C) 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#include "dictionaryrunner.h"
#include "dictionaryrunner_config.h"

#include <QApplication>
#include <QClipboard>
#include <KIcon>
#include <KDebug>

DictionaryRunner::DictionaryRunner(QObject *parent, const QVariantList &args)
    : AbstractRunner(parent, args),
    m_engine(nullptr)
{
    setSpeed(SlowSpeed);
    setPriority(LowPriority);
    setObjectName(QLatin1String("Dictionary"));
    setIgnoredTypes(
        Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
        Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Executable |
        Plasma::RunnerContext::ShellCommand
    );
}

void DictionaryRunner::init()
{
    reloadConfiguration();
    m_engine = dataEngine("dict");
}

void DictionaryRunner::reloadConfiguration()
{
    KConfigGroup c = config();
    m_triggerWord = c.readEntry(CONFIG_TRIGGERWORD, i18nc("Trigger word before word to define", "define"));
    if (!m_triggerWord.isEmpty()) {
        m_triggerWord.append(QLatin1Char(' '));
    }
    setSyntaxes(
        QList<Plasma::RunnerSyntax>()
            << Plasma::RunnerSyntax(Plasma::RunnerSyntax(i18nc("Dictionary keyword", "%1:q:", m_triggerWord), i18n("Finds the definition of :q:."))
        )
    );
}

void DictionaryRunner::match(Plasma::RunnerContext &context)
{
    if (!context.isValid()) {
        return;
    }
    QString query = context.query();
    if (!query.startsWith(m_triggerWord, Qt::CaseInsensitive)) {
        return;
    }
    query.remove(0, m_triggerWord.length());
    if (query.isEmpty()) {
        return;
    }

    Plasma::DataEngine::Data enginedata = m_engine->query(query);
    // qDebug() << Q_FUNC_INFO << query << enginedata;
    const QString enginedefinition = enginedata.value("definition").toString();
    if (!enginedefinition.isEmpty()) {
        const QString engineexample = enginedata.value("example").toString();
        Plasma::QueryMatch match(this);
        match.setText(enginedefinition);
        match.setSubtext(engineexample);
        match.setData(enginedefinition);
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIcon(KIcon(QLatin1String("accessories-dictionary")));
        context.addMatch(context.query(), match);
    }
}

void DictionaryRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString data = match.data().toString();
    QApplication::clipboard()->setText(data);
}

K_EXPORT_PLASMA_RUNNER(krunner_dictionary, DictionaryRunner)

#include "moc_dictionaryrunner.cpp"
