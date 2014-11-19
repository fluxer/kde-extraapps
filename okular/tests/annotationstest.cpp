/***************************************************************************
 *   Copyright (C) 2013 by Peter Grasch <me@bedahr.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qtest_kde.h>

#include "../core/document.h"
#include "../core/page.h"
#include "../core/annotations.h"
#include "../settings_core.h"

Q_DECLARE_METATYPE(Okular::Annotation*)
Q_DECLARE_METATYPE(Okular::LineAnnotation*)

class AnnotationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testDistance();
    void testDistance_data();
//     void testLine();
//     void testPoly();
//     void testInk();
//     void testHighlight();
//     void testGeom();
    void cleanupTestCase();

private:
    Okular::Document *m_document;
};

void AnnotationTest::initTestCase()
{
    Okular::SettingsCore::instance( "annotationtest" );
    m_document = new Okular::Document( 0 );
    const QString testFile = KDESRCDIR "data/file1.pdf";
    const KMimeType::Ptr mime = KMimeType::findByPath( testFile );
    m_document->openDocument(testFile, KUrl(), mime);
}

void AnnotationTest::cleanupTestCase()
{
    foreach ( Okular::Annotation* annotation, m_document->page( 0 )->annotations() )
        m_document->removePageAnnotation( 0, annotation );
}



void AnnotationTest::testDistance()
{
    QFETCH(Okular::Annotation*, annotation);
    QFETCH(double, x);
    QFETCH(double, y);
    QFETCH(int, distance);
    Okular::AnnotationObjectRect* rect = new Okular::AnnotationObjectRect( annotation );
    QCOMPARE( qRound( rect->distanceSqr( x, y, m_document->page(0)->width(), m_document->page(0)->height() ) ), distance );
    delete rect;
}

void AnnotationTest::testDistance_data()
{
    QTest::addColumn<Okular::Annotation*>("annotation");
    QTest::addColumn<double>("x");
    QTest::addColumn<double>("y");
    QTest::addColumn<int>("distance");

    double documentX = m_document->page(0)->width();
    double documentY = m_document->page(0)->height();

    // lines
    Okular::LineAnnotation *line = new Okular::LineAnnotation;
    line->setLinePoints( QLinkedList< Okular::NormalizedPoint >()
        << Okular::NormalizedPoint( 0.1, 0.1 )
        << Okular::NormalizedPoint( 0.9, 0.1 )
    );
    m_document->addPageAnnotation( 0, line );
    QTest::newRow("Line: Base point 1") << (Okular::Annotation*) line << 0.1 << 0.1 << 0;
    QTest::newRow("Line: Base point 2") << (Okular::Annotation*) line << 0.5 << 0.1 << 0;
    QTest::newRow("Line: Off by a lot") << (Okular::Annotation*) line << 0.5 << 0.5 << qRound( pow( 0.4 * documentY, 2 ) );
    QTest::newRow("Line: Off by a little") << (Okular::Annotation*) line << 0.1 << 0.1 + (5.0 /* px */ / documentY) << 25;


    // squares (non-filled squares also act as tests for the ink annotation as they share the path-distance code)
    for ( int i = 0; i < 2; ++i )
    {
        Okular::GeomAnnotation *square = new Okular::GeomAnnotation;
        square->setGeometricalType( Okular::GeomAnnotation::InscribedSquare );
        square->setBoundingRectangle( Okular::NormalizedRect( 0.1, 0.1, 0.9, 0.9 ) );
        if ( i != 0 )
            square->setGeometricalInnerColor( QColor(0, 0, 0) );
        m_document->addPageAnnotation( 0, square );
        QTest::newRow("Square: Base point 1") << (Okular::Annotation*) square << 0.1 << 0.1 << 0;
        QTest::newRow("Square: On edge 1") << (Okular::Annotation*) square << 0.1 << 0.2 << 0;
        QTest::newRow("Square: On edge 2") << (Okular::Annotation*) square << 0.2 << 0.1 << 0;
        QTest::newRow("Square: Inside") << (Okular::Annotation*) square << 0.2 << 0.2 <<
            ((i == 0) ? qRound( pow( 0.1 * documentX, 2 ) - 1 /* stroke width */) : 0 );
        QTest::newRow("Square: Outside") << (Okular::Annotation*) square << 0.0 << 0.0 <<
            qRound( pow( 0.1 * documentX, 2 ) + pow( 0.1 * documentY, 2) ) ;
    }

    // ellipsis
    for ( int i = 0; i < 2; ++i )
    {
        Okular::GeomAnnotation *ellipse = new Okular::GeomAnnotation;
        ellipse->setGeometricalType( Okular::GeomAnnotation::InscribedCircle );
        ellipse->setBoundingRectangle( Okular::NormalizedRect( 0.1, 0.1, 0.9, 0.5 ) );
        if ( i != 0 )
            ellipse->setGeometricalInnerColor( QColor(0, 0, 0) );
        m_document->addPageAnnotation( 0, ellipse );
        QTest::newRow("Ellipse: Base point 1") << (Okular::Annotation*) ellipse << 0.1 << 0.3 << 0;
        QTest::newRow("Ellipse: Inside") << (Okular::Annotation*) ellipse << 0.2 << 0.3 <<
            qRound( (i == 0) ? pow( documentX * 0.1, 2 ) - 1 /* pen */ : 0 );
        QTest::newRow("Ellipse: Outside") << (Okular::Annotation*) ellipse << 0.0 << 0.3 << qRound( pow( documentX * 0.1, 2 ) );
    }


    // highlight
    Okular::HighlightAnnotation *highlight = new Okular::HighlightAnnotation;
    Okular::HighlightAnnotation::Quad q;
    q.setPoint( Okular::NormalizedPoint( 0.1, 0.1 ), 0 );
    q.setPoint( Okular::NormalizedPoint( 0.2, 0.1 ), 1 );
    q.setPoint( Okular::NormalizedPoint( 0.8, 0.9 ), 2 );
    q.setPoint( Okular::NormalizedPoint( 0.9, 0.9 ), 3 );
    highlight->highlightQuads() << q;
    m_document->addPageAnnotation( 0, highlight );
    QTest::newRow("Highlight: Point 1") << (Okular::Annotation*) highlight << 0.1 << 0.1 << 0;
    QTest::newRow("Highlight: Point 2") << (Okular::Annotation*) highlight << 0.2 << 0.1 << 0;
    QTest::newRow("Highlight: Point 3") << (Okular::Annotation*) highlight << 0.8 << 0.9 << 0;
    QTest::newRow("Highlight: Point 4") << (Okular::Annotation*) highlight << 0.9 << 0.9 << 0;
    QTest::newRow("Highlight: Inside") << (Okular::Annotation*) highlight << 0.5 << 0.5 << 0;
    QTest::newRow("Highlight: Outside") << (Okular::Annotation*) highlight << 1.0 << 0.9 << qRound( pow( documentX * 0.1, 2 ) );
}


QTEST_KDEMAIN( AnnotationTest, GUI )

#include "annotationstest.moc"
