/***************************************************************************
 *   KKernelLog, a kernel log viewer tool                                  *
 *   Copyright (C) 2007 by Nicolas Ternisien                               *
 *   nicolas.ternisien@gmail.com                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include <QList>
#include <QStringList>
#include <QThread>

#include <qtest_kde.h>

#include <kurl.h>
#include <kdirwatch.h>

#include "testUtil.h"

#include "analyzer.h"
#include "globals.h"

#include "logLevel.h"
#include "logFile.h"
#include "logViewModel.h"
#include "logViewWidget.h"

#include "ksystemlogConfig.h"

#include "logging.h"

#include "kernelAnalyzer.h"
#include "localLogFileReader.h"

/**
 * Reimplements the Kernel Analyzer using a Local File Reader
 */
class KernelAnalyzerLocalReader : public KernelAnalyzer {
	public:
	
	KernelAnalyzerLocalReader(LogMode* logMode) :
		KernelAnalyzer(logMode) {

	}

	virtual ~KernelAnalyzerLocalReader() {

	}

	QDateTime findStartupTime() {
		return startupDateTime;
	}
	
	protected:
		LogFileReader* createLogFileReader(const LogFile& logFile) {
			return new LocalLogFileReader(logFile);
		}
	
};


class KernelAnalyzerTest : public QObject {

	Q_OBJECT
	
private slots:
	
	void initTestCase();
	
	void testUbuntuDmesg();
	void testSuseDmesg();

private:
	void compareWithMinTime(QList<LogLine*> lines, const QDateTime& minTime);
	
private:
	TestUtil testUtil;

};

void KernelAnalyzerTest::initTestCase() {
	testUtil.registerLogModeFactories();
}

void KernelAnalyzerTest::testUbuntuDmesg() {

	//Specifical configuration
	KSystemLogConfig::setMaxLines(1000);
	KSystemLogConfig::setDeleteDuplicatedLines(false);

	LogMode* logMode = Globals::instance()->findLogMode(QLatin1String("kernelLogMode"));
	KernelAnalyzerLocalReader* kernelAnalyzer = new KernelAnalyzerLocalReader(logMode);
	LogViewModel* model = testUtil.defineLogViewModel(kernelAnalyzer);
	
	QVERIFY(kernelAnalyzer);
	QVERIFY(model);

	QList<LogFile> logFiles = testUtil.createLogFiles(QLatin1String(":/testFiles/kernel/ubuntu.dmesg"));

	kernelAnalyzer->setLogFiles(logFiles);

	kernelAnalyzer->watchLogFiles(true);

	QCOMPARE(model->itemCount(), 25);
	QCOMPARE(model->isEmpty(), false);

	QList<LogLine*> logLines = model->logLines();

	QStringList items = QStringList() << QLatin1String("ADDRCONF(NETDEV_UP)") << QLatin1String("eth0: link is not ready");
	QDateTime assertedDateTime = kernelAnalyzer->findStartupTime();
	assertedDateTime = assertedDateTime.addSecs(22);
	assertedDateTime = assertedDateTime.addMSecs(232);

	testUtil.testLine(
			logLines.at(0), 
			logFiles.at(0).url().path(), 
			Globals::instance()->informationLogLevel(), 
			assertedDateTime, 
			items
	);

	testUtil.destroyReader(kernelAnalyzer);
}

void KernelAnalyzerTest::testSuseDmesg() {

	//Specifical configuration
	KSystemLogConfig::setMaxLines(1000);
	KSystemLogConfig::setDeleteDuplicatedLines(false);

	LogMode* logMode = Globals::instance()->findLogMode(QLatin1String("kernelLogMode"));
	KernelAnalyzerLocalReader* kernelAnalyzer = new KernelAnalyzerLocalReader(logMode);
	LogViewModel* model = testUtil.defineLogViewModel(kernelAnalyzer);
	
	QVERIFY(kernelAnalyzer);
	QVERIFY(model);

	QList<LogFile> logFiles = testUtil.createLogFiles(QLatin1String(":/testFiles/kernel/suse.dmesg"));

	kernelAnalyzer->setLogFiles(logFiles);

	kernelAnalyzer->watchLogFiles(true);

	QCOMPARE(model->itemCount(), 23);
	QCOMPARE(model->isEmpty(), false);

	QList<LogLine*> logLines = model->logLines();

	QStringList items = QStringList() << QLatin1String("r8169") << QLatin1String("eth0: link down");

	testUtil.testLine(
			logLines.at(0), 
			logFiles.at(0).url().path(), 
			Globals::instance()->informationLogLevel(), 
			kernelAnalyzer->findStartupTime(), 
			items
	);

	testUtil.destroyReader(kernelAnalyzer);
}


QTEST_KDEMAIN(KernelAnalyzerTest, GUI)

#include "kernelAnalyzerTest.moc"
