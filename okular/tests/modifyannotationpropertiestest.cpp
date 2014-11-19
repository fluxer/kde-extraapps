/***************************************************************************
 *   Copyright (C) 2013 by Jon Mease <jon.mease@gmail.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qtest_kde.h>
#include <kmimetype.h>
#include "../settings_core.h"
#include <core/area.h>
#include <core/annotations.h>
#include "core/document.h"
#include "testingutils.h"

static const QColor RED = QColor(255, 0, 0);
static const QColor GREEN = QColor(0, 255, 0.0);
static const QColor BLUE = QColor(0, 0, 255);

class ModifyAnnotationPropertiesTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void testModifyAnnotationProperties();
    void testModifyDefaultAnnotationProperties();
    void testModifyAnnotationPropertiesWithRotation_Bug318828();
private:
    Okular::Document *m_document;
    Okular::TextAnnotation *m_annot1;
};

void ModifyAnnotationPropertiesTest::initTestCase()
{
    Okular::SettingsCore::instance( "editannotationcontentstest" );
    m_document = new Okular::Document( 0 );
}

void ModifyAnnotationPropertiesTest::cleanupTestCase()
{
    delete m_document;
}

void ModifyAnnotationPropertiesTest::init()
{
    const QString testFile = KDESRCDIR "data/file1.pdf";
    const KMimeType::Ptr mime = KMimeType::findByPath( testFile );
    m_document->openDocument(testFile, KUrl(), mime);

    // Undo and Redo should be unavailable when docuemnt is first opened.
    QVERIFY( !m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Create two distinct text annotations
    m_annot1 = new Okular::TextAnnotation();
    m_annot1->setBoundingRectangle( Okular::NormalizedRect( 0.1, 0.1, 0.15, 0.15 ) );
    m_annot1->setContents( QString( "Hello, World" ) );
    m_annot1->setAuthor( "Jon Mease" );
    m_annot1->style().setColor( RED );
    m_annot1->style().setWidth( 4.0 );
    m_document->addPageAnnotation( 0, m_annot1 );
}

void ModifyAnnotationPropertiesTest::cleanup()
{
    m_document->closeDocument();
    // m_annot1 and m_annot2 are deleted when document is closed
}

void ModifyAnnotationPropertiesTest::testModifyAnnotationProperties()
{
    // Add m_annot1 to document and record its properties XML string
    QString origLine1Xml =  TestingUtils::getAnnotationXml( m_annot1 );

    // Tell document we're going to modify m_annot1's properties
    m_document->prepareToModifyAnnotationProperties( m_annot1 );

    // Now modify m_annot1's properties and record properties XML string
    m_annot1->style().setWidth( 8.0 );
    m_annot1->style().setColor( GREEN );
    m_document->modifyPageAnnotationProperties( 0, m_annot1 );
    QString m_annot1XmlA =  TestingUtils::getAnnotationXml( m_annot1 );
    QCOMPARE( 8.0, m_annot1->style().width() );
    QCOMPARE( GREEN, m_annot1->style().color() );

    // undo modification and check that original properties have been restored
    m_document->undo();
    QCOMPARE( 4.0, m_annot1->style().width() );
    QCOMPARE( RED, m_annot1->style().color() );
    QCOMPARE( origLine1Xml, TestingUtils::getAnnotationXml( m_annot1 ) );

    // redo modification and verify that new properties have been restored
    m_document->redo();
    QCOMPARE( 8.0, m_annot1->style().width() );
    QCOMPARE( GREEN, m_annot1->style().color() );
    QCOMPARE( m_annot1XmlA, TestingUtils::getAnnotationXml( m_annot1 ) );

    // Verify that default values are properly restored.  (We haven't explicitly set opacity yet)
    QCOMPARE( 1.0, m_annot1->style().opacity() );
    m_document->prepareToModifyAnnotationProperties( m_annot1 );
    m_annot1->style().setOpacity( 0.5 );
    m_document->modifyPageAnnotationProperties( 0, m_annot1 );
    QCOMPARE( 0.5, m_annot1->style().opacity() );

    m_document->undo();
    QCOMPARE( 1.0, m_annot1->style().opacity() );
    QCOMPARE( m_annot1XmlA, TestingUtils::getAnnotationXml( m_annot1 ) );

    // And finally undo back to original properties
    m_document->undo();
    QCOMPARE( 4.0, m_annot1->style().width() );
    QCOMPARE( RED, m_annot1->style().color() );
    QCOMPARE( origLine1Xml, TestingUtils::getAnnotationXml( m_annot1 ) );
}

void ModifyAnnotationPropertiesTest::testModifyDefaultAnnotationProperties()
{
    QString origLine1Xml =  TestingUtils::getAnnotationXml( m_annot1 );

    // Verify that default values are properly restored.  (We haven't explicitly set opacity yet)
    QCOMPARE( 1.0, m_annot1->style().opacity() );
    m_document->prepareToModifyAnnotationProperties( m_annot1 );
    m_annot1->style().setOpacity( 0.5 );
    m_document->modifyPageAnnotationProperties( 0, m_annot1 );
    QCOMPARE( 0.5, m_annot1->style().opacity() );

    m_document->undo();
    QCOMPARE( 1.0, m_annot1->style().opacity() );
    QCOMPARE( origLine1Xml, TestingUtils::getAnnotationXml( m_annot1 ) );
}

void ModifyAnnotationPropertiesTest::testModifyAnnotationPropertiesWithRotation_Bug318828()
{
    Okular::NormalizedRect boundingRect = Okular::NormalizedRect( 0.1, 0.1, 0.15, 0.15 );
    Okular::NormalizedRect transformedBoundingRect;
    m_annot1->setBoundingRectangle( boundingRect );
    m_document->addPageAnnotation( 0, m_annot1 );

    transformedBoundingRect = m_annot1->transformedBoundingRectangle();

    // Before page rotation boundingRect and transformedBoundingRect should be equal
    QCOMPARE( boundingRect, transformedBoundingRect );
    m_document->setRotation( 1 );

    // After rotation boundingRect should remain unchanged but
    // transformedBoundingRect should no longer equal boundingRect
    QCOMPARE( boundingRect, m_annot1->boundingRectangle() );
    transformedBoundingRect = m_annot1->transformedBoundingRectangle();
    QVERIFY( !( boundingRect == transformedBoundingRect ) );

    // Modifying the properties of m_annot1 while page is rotated shouldn't
    // alter either boundingRect or transformedBoundingRect
    m_document->prepareToModifyAnnotationProperties( m_annot1 );
    m_annot1->style().setOpacity( 0.5 );
    m_document->modifyPageAnnotationProperties( 0, m_annot1 );

    QCOMPARE( boundingRect, m_annot1->boundingRectangle() );
    QCOMPARE( transformedBoundingRect, m_annot1->transformedBoundingRectangle() );
}

QTEST_KDEMAIN( ModifyAnnotationPropertiesTest, GUI )
#include "modifyannotationpropertiestest.moc"
