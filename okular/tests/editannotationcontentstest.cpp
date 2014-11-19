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
#include "core/annotations.h"
#include "core/document.h"

class MockEditor;

class EditAnnotationContentsTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void testConsecutiveCharBackspacesMerged();
    void testConsecutiveNewlineBackspacesNotMerged();
    void testConsecutiveCharInsertionsMerged();
    void testConsecutiveNewlineInsertionsNotMerged();
    void testConsecutiveCharDeletesMerged();
    void testConsecutiveNewlineDeletesNotMerged();
    void testConsecutiveEditsNotMergedAcrossDifferentAnnotations();
    void testInsertWithSelection();
    void testCombinations();

private:
    Okular::Document *m_document;
    Okular::TextAnnotation *m_annot1;
    Okular::TextAnnotation *m_annot2;
    MockEditor *m_editor1;
    MockEditor *m_editor2;
};

/*
 * Simple class that receives the Document::annotationContentsChangedByUndoRedo
 * signal that would normally be directed to an annotation's
 * contents editor (For example AnnotWindow)
 */
class MockEditor: public QObject
{
    Q_OBJECT

public:
    MockEditor( Okular::Annotation *annot, Okular::Document *doc);
    QString contents() { return m_contents; }
    int cursorPos() { return m_cursorPos; }
    int anchorPos() { return m_anchorPos; }

private slots:
    void slotAnnotationContentsChangedByUndoRedo( Okular::Annotation* annotation, const QString & contents, int cursorPos, int anchorPos );

private:
    Okular::Document *m_document;
    Okular::Annotation *m_annot;

    QString m_contents;
    int m_cursorPos;
    int m_anchorPos;
};

MockEditor::MockEditor( Okular::Annotation *annot, Okular::Document *doc )
{
    m_annot = annot;
    m_document = doc;
    connect( m_document, SIGNAL(annotationContentsChangedByUndoRedo(Okular::Annotation*,QString,int,int)),
             this, SLOT(slotAnnotationContentsChangedByUndoRedo(Okular::Annotation*,QString,int,int)));
    m_cursorPos = 0;
    m_anchorPos = 0;
    m_contents = annot->contents();
}

void MockEditor::slotAnnotationContentsChangedByUndoRedo(Okular::Annotation* annotation, const QString& contents, int cursorPos, int anchorPos)
{
    if( annotation == m_annot )
    {
        m_contents = contents;
        m_cursorPos = cursorPos;
        m_anchorPos = anchorPos;
    }
}

void EditAnnotationContentsTest::initTestCase()
{
    Okular::SettingsCore::instance( "editannotationcontentstest" );
    m_document = new Okular::Document( 0 );
}

void EditAnnotationContentsTest::cleanupTestCase()
{
    delete m_document;
}

void EditAnnotationContentsTest::init()
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
    m_document->addPageAnnotation( 0, m_annot1 );

    m_annot2 = new Okular::TextAnnotation();
    m_annot2->setBoundingRectangle( Okular::NormalizedRect( 0.1, 0.1, 0.15, 0.15 ) );
    m_annot2->setContents( QString( "Hello, World" ) );
    m_document->addPageAnnotation( 0, m_annot2 );

    // setup editors
    m_editor1 = new MockEditor( m_annot1, m_document );
    m_editor2 = new MockEditor( m_annot2, m_document );
}

void EditAnnotationContentsTest::cleanup()
{
    m_document->closeDocument();
    delete m_editor1;
    delete m_editor2;
    // m_annot1 and m_annot2 are deleted when document is closed
}

void EditAnnotationContentsTest::testConsecutiveCharBackspacesMerged()
{
    // Hello, World| -> Hello, Worl|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Worl" ), 11, 12, 12 );
    QCOMPARE( QString( "Hello, Worl" ), m_annot1->contents() );

    // Hello, Worl| -> Hello, Wor|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Wor" ), 10, 11, 11 );
    QCOMPARE( QString( "Hello, Wor" ), m_annot1->contents() );

    // undo and verify that consecutive backspace operations are merged together
    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( 12, m_editor1->cursorPos() );
    QCOMPARE( 12, m_editor1->anchorPos() );

    m_document->redo();
    QCOMPARE( QString( "Hello, Wor" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, Wor" ), m_editor1->contents() );
    QCOMPARE( 10, m_editor1->cursorPos() );
    QCOMPARE( 10, m_editor1->anchorPos() );

    // Hello, Wor| -> Hello, Wo|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Wo" ), 9, 10, 10 );
    QCOMPARE( QString( "Hello, Wo" ), m_annot1->contents() );

    // Hello, Wo| -> Hello, W|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, W" ), 8, 9, 9 );
    QCOMPARE( QString( "Hello, W" ), m_annot1->contents() );

    // Hello, W| -> Hello, |
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, " ), 7, 8, 8 );
    QCOMPARE( QString( "Hello, " ), m_annot1->contents() );

    // undo and verify that consecutive backspace operations are merged together
    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( 12, m_editor1->cursorPos() );
    QCOMPARE( 12, m_editor1->anchorPos() );

    m_document->redo();
    QCOMPARE( QString( "Hello, " ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, " ), m_editor1->contents() );
    QCOMPARE( 7, m_editor1->cursorPos() );
    QCOMPARE( 7, m_editor1->anchorPos() );
}

void EditAnnotationContentsTest::testConsecutiveNewlineBackspacesNotMerged()
{
    // Set contents to Hello, \n\n|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, \n\nWorld" ), 0, 0, 0 );
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_annot1->contents() );

    // Hello, \n\n|World -> Hello, \n|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, \nWorld" ), 8, 9, 9 );
    QCOMPARE( QString( "Hello, \nWorld" ), m_annot1->contents() );

    // Hello, \n|World -> Hello, |World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, World" ), 7, 8, 8 );
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );

    // Hello, |World -> Hello,|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello,World" ), 6, 7, 7 );
    QCOMPARE( QString( "Hello,World" ), m_annot1->contents() );

    // Hello,|World -> Hello|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "HelloWorld" ), 5, 6, 6 );
    QCOMPARE( QString( "HelloWorld" ), m_annot1->contents() );

    // Backspace operations of non-newline characters should be merged
    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( 7, m_editor1->cursorPos() );
    QCOMPARE( 7, m_editor1->anchorPos() );

    // Backspace operations on newline characters should not be merged
    m_document->undo();
    QCOMPARE( QString( "Hello, \nWorld" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, \nWorld" ), m_editor1->contents() );
    QCOMPARE( 8, m_editor1->cursorPos() );
    QCOMPARE( 8, m_editor1->anchorPos() );

    m_document->undo();
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_editor1->contents() );
    QCOMPARE( 9, m_editor1->cursorPos() );
    QCOMPARE( 9, m_editor1->anchorPos() );
}

void EditAnnotationContentsTest::testConsecutiveCharInsertionsMerged()
{
    // Hello, |World -> Hello, B|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, BWorld" ), 8, 7, 7 );
    QCOMPARE( QString( "Hello, BWorld" ), m_annot1->contents() );

    // Hello, l| -> Hello, li|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, BiWorld" ), 9, 8, 8 );
    QCOMPARE( QString( "Hello, BiWorld" ), m_annot1->contents() );

    // Hello, li| -> Hello, lin|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, BigWorld" ), 10, 9, 9 );
    QCOMPARE( QString( "Hello, BigWorld" ), m_annot1->contents() );

    // Hello, lin| -> Hello, line|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Big World" ), 11, 10, 10 );
    QCOMPARE( QString( "Hello, Big World" ), m_annot1->contents() );

    // Verify undo/redo operations merged
    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( 7, m_editor1->cursorPos() );
    QCOMPARE( 7, m_editor1->anchorPos() );

    m_document->redo();
    QCOMPARE( QString( "Hello, Big World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, Big World" ), m_editor1->contents() );
    QCOMPARE( 11, m_editor1->cursorPos() );
    QCOMPARE( 11, m_editor1->anchorPos() );
}

void EditAnnotationContentsTest::testConsecutiveNewlineInsertionsNotMerged()
{
    // Hello, |World -> Hello, \n|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, \nWorld" ), 8, 7, 7 );
    QCOMPARE( QString( "Hello, \nWorld" ), m_annot1->contents() );

    // Hello, |World -> Hello, \n|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, \n\nWorld" ), 9, 8, 8 );
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_annot1->contents() );

    m_document->undo();
    QCOMPARE( QString( "Hello, \nWorld" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, \nWorld" ), m_editor1->contents() );
    QCOMPARE( 8, m_editor1->cursorPos() );
    QCOMPARE( 8, m_editor1->anchorPos() );

    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( 7, m_editor1->cursorPos() );
    QCOMPARE( 7, m_editor1->anchorPos() );

    m_document->redo();
    QCOMPARE( QString( "Hello, \nWorld" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, \nWorld" ), m_editor1->contents() );
    QCOMPARE( 8, m_editor1->cursorPos() );
    QCOMPARE( 8, m_editor1->anchorPos() );

    m_document->redo();
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_editor1->contents() );
    QCOMPARE( 9, m_editor1->cursorPos() );
    QCOMPARE( 9, m_editor1->anchorPos() );
}

void EditAnnotationContentsTest::testConsecutiveCharDeletesMerged()
{
    // Hello, |World -> Hello, |orld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, orld" ), 7, 7, 7 );
    QCOMPARE( QString( "Hello, orld" ), m_annot1->contents() );

    // Hello, |orld -> Hello, |rld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, rld" ), 7, 7, 7 );
    QCOMPARE( QString( "Hello, rld" ), m_annot1->contents() );

    // Hello, |rld -> Hello, |ld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, ld" ), 7, 7, 7 );
    QCOMPARE( QString( "Hello, ld" ), m_annot1->contents() );

    // Hello, |ld -> Hello, |d
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, d" ), 7, 7, 7 );
    QCOMPARE( QString( "Hello, d" ), m_annot1->contents() );

    // Hello, | -> Hello, |
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, " ), 7, 7, 7 );
    QCOMPARE( QString( "Hello, " ), m_annot1->contents() );

    // Verify undo/redo operations merged
    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( 7, m_editor1->cursorPos() );
    QCOMPARE( 7, m_editor1->anchorPos() );

    m_document->redo();
    QCOMPARE( QString( "Hello, " ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, " ), m_editor1->contents() );
    QCOMPARE( 7, m_editor1->cursorPos() );
    QCOMPARE( 7, m_editor1->anchorPos() );
}

void EditAnnotationContentsTest::testConsecutiveNewlineDeletesNotMerged()
{
    // Set contents to Hello, \n\n|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, \n\nWorld" ), 0, 0, 0 );
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_annot1->contents() );

    // He|llo, \n\nWorld ->  He|lo, \n\nWorld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Helo, \n\nWorld" ), 2, 2, 2 );
    QCOMPARE( QString( "Helo, \n\nWorld" ), m_annot1->contents() );

    // He|lo, \n\nWorld ->  He|o, \n\nWorld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Heo, \n\nWorld" ), 2, 2, 2 );
    QCOMPARE( QString( "Heo, \n\nWorld" ), m_annot1->contents() );

    // He|o, \n\nWorld ->  He|, \n\nWorld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "He, \n\nWorld" ), 2, 2, 2 );
    QCOMPARE( QString( "He, \n\nWorld" ), m_annot1->contents() );

    // He|, \n\nWorld ->  He| \n\nWorld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "He \n\nWorld" ), 2, 2, 2 );
    QCOMPARE( QString( "He \n\nWorld" ), m_annot1->contents() );

    // He| \n\nWorld ->  He|\n\nWorld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "He\n\nWorld" ), 2, 2, 2 );
    QCOMPARE( QString( "He\n\nWorld" ), m_annot1->contents() );

    // He|\n\nWorld ->  He|\nWorld
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "He\nWorld" ), 2, 2, 2 );
    QCOMPARE( QString( "He\nWorld" ), m_annot1->contents() );

    // He|\nWorld ->  He|World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "HeWorld" ), 2, 2, 2 );
    QCOMPARE( QString( "HeWorld" ), m_annot1->contents() );

    // Verify that deletions of newlines are not merged, but deletions of other characters are
    m_document->undo();
    QCOMPARE( QString( "He\nWorld" ), m_annot1->contents() );
    QCOMPARE( QString( "He\nWorld" ), m_editor1->contents() );
    QCOMPARE( 2, m_editor1->cursorPos() );
    QCOMPARE( 2, m_editor1->anchorPos() );

    m_document->undo();
    QCOMPARE( QString( "He\n\nWorld" ), m_annot1->contents() );
    QCOMPARE( QString( "He\n\nWorld" ), m_editor1->contents() );
    QCOMPARE( 2, m_editor1->cursorPos() );
    QCOMPARE( 2, m_editor1->anchorPos() );

    m_document->undo();
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, \n\nWorld" ), m_editor1->contents() );
    QCOMPARE( 2, m_editor1->cursorPos() );
    QCOMPARE( 2, m_editor1->anchorPos() );
}

void EditAnnotationContentsTest::testConsecutiveEditsNotMergedAcrossDifferentAnnotations()
{
    // Annot1: Hello, World| -> Hello, Worl|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Worl" ), 11, 12, 12 );
    QCOMPARE( QString( "Hello, Worl" ), m_annot1->contents() );
    // Annot1: Hello, Worl| -> Hello, Wor|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Wor" ), 10, 11, 11 );
    QCOMPARE( QString( "Hello, Wor" ), m_annot1->contents() );

    // Annot2: Hello, World| -> Hello, Worl|
    m_document->editPageAnnotationContents( 0, m_annot2, QString( "Hello, Worl" ), 11, 12, 12 );
    QCOMPARE( QString( "Hello, Worl" ), m_annot2->contents() );
    // Annot2: Hello, Worl| -> Hello, Wor|
    m_document->editPageAnnotationContents( 0, m_annot2, QString( "Hello, Wor" ), 10, 11, 11 );
    QCOMPARE( QString( "Hello, Wor" ), m_annot2->contents() );

    // Annot1: Hello, Wor| -> Hello, Wo|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Wo" ), 9, 10, 10 );
    QCOMPARE( QString( "Hello, Wo" ), m_annot1->contents() );
    // Annot1: Hello, Wo| -> Hello, W|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, W" ), 8, 9, 9 );
    QCOMPARE( QString( "Hello, W" ), m_annot1->contents() );
    // Annot1: Hello, W| -> Hello, |
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, " ), 7, 8, 8 );
    QCOMPARE( QString( "Hello, " ), m_annot1->contents() );

    // Annot2: Hello, Wor| -> Hello, Wo|
    m_document->editPageAnnotationContents( 0, m_annot2, QString( "Hello, Wo" ), 9, 10, 10 );
    QCOMPARE( QString( "Hello, Wo" ), m_annot2->contents() );
    // Annot2: Hello, Wo| -> Hello, W|
    m_document->editPageAnnotationContents( 0, m_annot2, QString( "Hello, W" ), 8, 9, 9 );
    QCOMPARE( QString( "Hello, W" ), m_annot2->contents() );
    // Annot2: Hello, W| -> Hello, |
    m_document->editPageAnnotationContents( 0, m_annot2, QString( "Hello, " ), 7, 8, 8 );
    QCOMPARE( QString( "Hello, " ), m_annot2->contents() );


    // undo and verify that consecutive backspace operations are merged together
    // m_annot2 -> "Hello, Wor|"
    m_document->undo();
    QCOMPARE( QString( "Hello, Wor" ), m_annot2->contents() );
    QCOMPARE( QString( "Hello, " ), m_editor1->contents() );
    QCOMPARE( QString( "Hello, Wor" ), m_editor2->contents() );
    QCOMPARE( 10, m_editor2->cursorPos() );
    QCOMPARE( 10, m_editor2->anchorPos() );

    // m_annot1 -> "Hello, Wor|"
    m_document->undo();
    QCOMPARE( QString( "Hello, Wor" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, Wor" ), m_editor1->contents() );
    QCOMPARE( QString( "Hello, Wor" ), m_editor2->contents() );
    QCOMPARE( 10, m_editor1->cursorPos() );
    QCOMPARE( 10, m_editor1->anchorPos() );

    // m_annot2 -> "Hello, World|"
    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot2->contents() );
    QCOMPARE( QString( "Hello, Wor" ), m_editor1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor2->contents() );
    QCOMPARE( 12, m_editor2->cursorPos() );
    QCOMPARE( 12, m_editor2->anchorPos() );

    // m_annot1 -> "Hello, World|"
    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor2->contents() );
    QCOMPARE( 12, m_editor1->cursorPos() );
    QCOMPARE( 12, m_editor1->anchorPos() );
}

void EditAnnotationContentsTest::testInsertWithSelection()
{
    // Annot1: |Hello|, World -> H|, World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "H, World" ), 1, 0, 5 );
    QCOMPARE( QString( "H, World" ), m_annot1->contents() );

    // Annot1: H|, World -> Hi|, World
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hi, World" ), 2, 1, 1 );
    QCOMPARE( QString( "Hi, World" ), m_annot1->contents() );

    m_document->undo();
    QCOMPARE( QString( "H, World" ), m_annot1->contents() );
    QCOMPARE( QString( "H, World" ), m_editor1->contents() );
    QCOMPARE( 1, m_editor1->cursorPos() );
    QCOMPARE( 1, m_editor1->anchorPos() );

    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( 0, m_editor1->cursorPos() );
    QCOMPARE( 5, m_editor1->anchorPos() );
}

void EditAnnotationContentsTest::testCombinations()
{
    // Annot1: Hello, World| -> Hello, Worl|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Worl" ), 11, 12, 12 );
    QCOMPARE( QString( "Hello, Worl" ), m_annot1->contents() );

    // Annot1: Hello, Worl| -> Hello, Wor|
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "Hello, Wor" ), 10, 11, 11 );
    QCOMPARE( QString( "Hello, Wor" ), m_annot1->contents() );

    // Annot1: |He|llo, Wor -> |llo, Wor
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "llo, Wor" ), 0, 2, 0 );
    QCOMPARE( QString( "llo, Wor" ), m_annot1->contents() );

    // Annot1: |llo, Wor -> |lo, Wor
    m_document->editPageAnnotationContents( 0, m_annot1, QString( "lo, Wor" ), 0, 0, 0 );
    QCOMPARE( QString( "lo, Wor" ), m_annot1->contents() );

    m_document->undo();
    QCOMPARE( QString( "llo, Wor" ), m_annot1->contents() );
    QCOMPARE( QString( "llo, Wor" ), m_editor1->contents() );
    QCOMPARE( 0, m_editor1->cursorPos() );
    QCOMPARE( 0, m_editor1->anchorPos() );

    m_document->undo();
    QCOMPARE( QString( "Hello, Wor" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, Wor" ), m_editor1->contents() );
    QCOMPARE( 2, m_editor1->cursorPos() );
    QCOMPARE( 0, m_editor1->anchorPos() );

    m_document->undo();
    QCOMPARE( QString( "Hello, World" ), m_annot1->contents() );
    QCOMPARE( QString( "Hello, World" ), m_editor1->contents() );
    QCOMPARE( 12, m_editor1->cursorPos() );
    QCOMPARE( 12, m_editor1->anchorPos() );
}

QTEST_KDEMAIN( EditAnnotationContentsTest, GUI )
#include "editannotationcontentstest.moc"
