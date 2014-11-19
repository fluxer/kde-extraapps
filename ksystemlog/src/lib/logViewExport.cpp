/***************************************************************************
 *   KSystemLog, a system log viewer tool                                  *
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

#include "logViewExport.h"

#include <QPainter>
#include <QClipboard>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>

#include <kapplication.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kfilterdev.h>
#include <kdeprintdialog.h>

#include "logging.h"

#include "logViewWidgetItem.h"
#include "logViewWidget.h"
#include "logLine.h"

#include "levelPrintPage.h"

LogViewExport::LogViewExport(QWidget* parent, LogViewWidget* logViewWidget) :
	parent(parent), logViewWidget(logViewWidget) {

}

LogViewExport::~LogViewExport() {

}

void LogViewExport::sendMail() {
	logDebug() << "Exporting to mail..." << endl;

	QString body(i18n("Here are my logs:\n"));

	body+=i18n("---------------------------------------\n");

	int i=0;
	QTreeWidgetItemIterator it(logViewWidget, QTreeWidgetItemIterator::Selected);
	while ( *it != NULL) {
		LogViewWidgetItem* item=static_cast<LogViewWidgetItem*> (*it);

		body+= item->logLine()->exportToText();
		body+=QLatin1Char( '\n' );

		++it;
		++i;
	}

	body+=i18n("---------------------------------------\n");

	//Too much lines selected
	if (i>1000) {
		KMessageBox::sorry(parent, i18n("You have selected too many lines. Please only select important log lines."), i18n("Too Many Lines Selected"));
		return;
	}

	// Parameters list of this method
	//const QString &   to,
	//const QString &   cc,
	//const QString &   bcc,
	//const QString &   subject,
	//const QString &   body,
	//const QString &   messageFile,
	//const QStringList &   attachURLs,
	//const QCString &   startup_id
	KToolInvocation::invokeMailer(QLatin1String( "" ), QLatin1String( "" ), QLatin1String( "" ), i18n("Log Lines of my problem"), body, QLatin1String( "" ), QStringList(), kapp->startupId());
}

void LogViewExport::printSelection() {
	logDebug() << "Printing selection..." << endl;

	QPrinter printer;

	// do some printer initialization
	printer.setFullPage( true);

	/*
	 LevelPrintPage* dialogPage = new LevelPrintPage(parent);
	 printer.addDialogPage(dialogPage);
	 */

	// initialize the printer using the print dialog
	QPrintDialog *printDialog = KdePrint::createPrintDialog(&printer, parent);
	if (printDialog->exec() == false) {
		delete printDialog;
		return;
	}
	delete printDialog;

	// create a painter to paint on the printer object
	QPainter painter;

	// start painting
	painter.begin( &printer );

	QPen originalPen(painter.pen());

	int dpiy = painter.device()->logicalDpiY();
	int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
	QRect printView(margin, margin, painter.device()->width() - 2*margin, painter.device()->height() - 2*margin );

	int page = 1;

	int i = 0;
	int movement = 0;

	QTreeWidgetItemIterator it(logViewWidget, QTreeWidgetItemIterator::Selected);
	while ( *it != NULL) {
		LogViewWidgetItem* item=static_cast<LogViewWidgetItem*> (*it);

		/*
		 if(qtItem==NULL)
		 {
		 painter.setPen(originalPen);
		 printPageNumber(painter, printView, movement, page);
		 break;
		 }*/

		QString body = item->logLine()->exportToText();
		//body+= "\n";

		/* QPrinter Port: comment out as dialog page is not currently being used, so not ported
		QString strUseColor = printer.option("kde-ksystemlog-print_"+ item->logLine()->logLevel()->name());
		int use = strUseColor.toInt();
		if (use) {
			QPen pen(originalPen);
			pen.setColor(item->logLine()->logLevel()->color());
			painter.setPen(pen);
		} else {*/
			painter.setPen(originalPen);
		//}

		painter.drawText(printView, Qt::AlignLeft | Qt::TextWordWrap, body);

		int fontHeight = painter.fontMetrics().height();
		int lines = painter.fontMetrics().width(body) / printView.width() + 1;
		int moveBy = (fontHeight + 2) * lines;
		painter.translate(0, moveBy);

		movement = movement + moveBy;
		if (movement + margin >= printView.height()) {
			painter.setPen(originalPen);
			printPageNumber(painter, printView, movement, page);
			printer.newPage();
			page++;
			movement = 0;
		}

		++it;
		++i;
	}

	// stop painting, this will automatically send the print data to the printer
	painter.end();

}

void LogViewExport::printPageNumber(QPainter& painter, QRect& printView, int movement, int page) {
	logDebug() << "Printing page number..." << endl;

	painter.translate(0, -movement);
	printView.moveTo(QPoint(0, printView.height()) );
	painter.translate( 0, -printView.height() );
	painter.drawText(printView.right() - painter.fontMetrics().width(QString::number(page) ), printView.bottom()+ painter.fontMetrics().ascent() + 5, QString::number(page) );

}

void LogViewExport::copyToClipboard() {
	logDebug() << "Copying to clipboard..." << endl;

	int nbCopied=0;
	QString text;

	QTreeWidgetItemIterator it(logViewWidget, QTreeWidgetItemIterator::Selected);
	while ( *it != NULL) {
		LogViewWidgetItem* item=static_cast<LogViewWidgetItem*> (*it);

		//Copy the item content to the text string
		text.append(item->logLine()->exportToText());
		text.append(QLatin1Char( '\n' ));

		it++;
		nbCopied++;

	}

	//Copy text value only if it is not empty
	if (nbCopied==0) {
		emit statusBarChanged(i18n("No items selected. Nothing copied to clipboard."));
	} else {
		//Copy both to clipboard and X11-selection
		QApplication::clipboard()->setText(text, QClipboard::Clipboard);
		QApplication::clipboard()->setText(text, QClipboard::Selection);

		emit statusBarChanged(i18np("1 log line copied to clipboard.", "%1 log lines copied to clipboard.", nbCopied));
	}

	logDebug() << "Copied " << nbCopied << " log lines" << endl;

}

void LogViewExport::fileSave() {
	logDebug() << "Saving to a file..." << endl;

	QTreeWidgetItemIterator it(logViewWidget, QTreeWidgetItemIterator::Selected);

	//No item selected
	if ( *it == NULL) {
		emit statusBarChanged(i18n("No items selected. Please select items to be able to save them."));
		return;
	}

	QString filename = KFileDialog::getSaveFileName(KUrl(), QString(), parent);
	if (filename.isEmpty() == true) {
		return;
	}

	QIODevice* ioDev = KFilterDev::deviceForFile(filename);
	if (ioDev->open(QIODevice::WriteOnly)) {
		QTextStream stream(ioDev);

		int nbCopied=0;

		while ( *it != NULL) {
			LogViewWidgetItem* item=static_cast<LogViewWidgetItem*> (*it);

			//Copy the item content to the stream
			stream << item->logLine()->exportToText() << '\n';

			//Retrieve the next item
			it++;
			nbCopied++;

		}

		ioDev->close();

		delete ioDev;

		emit statusBarChanged(i18np("1 log line saved to '%2'.", "%1 log lines saved to '%2'.", nbCopied, filename));
	}
	else {
		QString message(i18n("Unable to save file '%1': Permission Denied.", filename));
		KMessageBox::error(parent, message, i18n("Unable to save file."));
	}


}

#include "logViewExport.moc"
