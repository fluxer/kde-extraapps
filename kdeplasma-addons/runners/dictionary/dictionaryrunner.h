/*
 * Copyright (C) 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYRUNNER_H
#define DICTIONARYRUNNER_H

#include <Plasma/AbstractRunner>
#include <Plasma/DataEngine>

class DictionaryRunner : public Plasma::AbstractRunner
{
    Q_OBJECT
public:
    DictionaryRunner(QObject *parent, const QVariantList &args);

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);
    void reloadConfiguration();

private:
    QString m_triggerWord;
    Plasma::DataEngine *m_engine;

protected slots:
    void init();
};

#endif // DICTIONARYRUNNER_H
