/***************************************************************************
 *   Copyright (C) 2013 by Fabio D'Urso <fabiodurso@hotmail.it>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qtest_kde.h>

#include "../generator_kimgio.h"
#include "../../settings_core.h"

#include <core/observer.h>
#include <core/page.h>
#include <ui/pagepainter.h>

#include <KTempDir>
#include <QImage>
#include <QPainter>
#include <QTemporaryFile>

class KIMGIOTest
: public QObject
{
	Q_OBJECT

	private slots:
		void testExifOrientation_data();
		void testExifOrientation();
};

// The following images have different Exif orientation tags, but they all
// are a 3x2 rectangle whose top-left pixel is black, and whose other pixels are
// white. Note that, due to JPEG lossy compression, some pixels are not pure
// white. In testExifOrientation, we only check the top-left and bottom-right
// corners.
void KIMGIOTest::testExifOrientation_data()
{
	QTest::addColumn<QString>( "imgPath" );

	// No Exif metadata at all
	QTest::newRow( "No Exif metadata" ) << KDESRCDIR "tests/data/testExifOrientation-noexif.jpg";

	// No Exif orientation information
	QTest::newRow( "Unspecified" ) << KDESRCDIR "tests/data/testExifOrientation-unspecified.jpg";

	// Valid Orientation values
	QTest::newRow( "Horizontal (normal)" ) << KDESRCDIR "tests/data/testExifOrientation-0.jpg";
	QTest::newRow( "Mirror horizontal" ) << KDESRCDIR "tests/data/testExifOrientation-0mirror.jpg";
	QTest::newRow( "Rotate 90 CW" ) << KDESRCDIR "tests/data/testExifOrientation-90.jpg";
	QTest::newRow( "Mirror horizontal and rotate 90 CW" ) << KDESRCDIR "tests/data/testExifOrientation-90mirror.jpg";
	QTest::newRow( "Rotate 180" ) << KDESRCDIR "tests/data/testExifOrientation-180.jpg";
	QTest::newRow( "Mirror vertical" ) << KDESRCDIR "tests/data/testExifOrientation-180mirror.jpg";
	QTest::newRow( "Rotate 270 CW" ) << KDESRCDIR "tests/data/testExifOrientation-270.jpg";
	QTest::newRow( "Mirror horizontal and rotate 270 CW" ) << KDESRCDIR "tests/data/testExifOrientation-270mirror.jpg";
}

void KIMGIOTest::testExifOrientation()
{
	QFETCH( QString, imgPath );

	Okular::SettingsCore::instance( "kimgiotest" );
	Okular::Document *m_document = new Okular::Document( 0 );
	const KMimeType::Ptr mime = KMimeType::findByPath( imgPath );

	Okular::DocumentObserver *dummyDocumentObserver = new Okular::DocumentObserver();
	m_document->addObserver( dummyDocumentObserver );

	// Load image
	m_document->openDocument( imgPath, KUrl(), mime );
	m_document->setRotation( 0 ); // Test the default rotation
	QCOMPARE( m_document->pages(), 1u );

	// Check size
	QCOMPARE( m_document->page(0)->width(), double(3) );
	QCOMPARE( m_document->page(0)->height(), double(2) );

	// Generate pixmap
	Okular::PixmapRequest *req = new Okular::PixmapRequest( dummyDocumentObserver, 0, 3, 2,
		1, Okular::PixmapRequest::NoFeature );
	m_document->requestPixmaps( QLinkedList<Okular::PixmapRequest*>() << req );
	QVERIFY( m_document->page(0)->hasPixmap( dummyDocumentObserver, 3, 2 ) );

	// Obtain image
	QImage img( 3, 2, QImage::Format_ARGB32_Premultiplied );
	QPainter p( &img );
	PagePainter::paintPageOnPainter( &p, m_document->page(0), dummyDocumentObserver, 0, 3, 2, QRect(0, 0, 3, 2) );

	// Verify pixel data
	QCOMPARE( img.pixel(0, 0), qRgb(0, 0, 0) );
	QCOMPARE( img.pixel(2, 1), qRgb(255, 255, 255) );

	m_document->removeObserver( dummyDocumentObserver );
	delete dummyDocumentObserver;
	delete m_document;
}

QTEST_KDEMAIN( KIMGIOTest, GUI )
#include "kimgiotest.moc"
