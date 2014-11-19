/***************************************************************************
 *   Copyright (C) 2013 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <qtest_kde.h>

#include "../core/document.h"
#include "../core/page.h"
#include "../core/textpage.h"
#include "../settings_core.h"

Q_DECLARE_METATYPE(Okular::Document::SearchStatus)

class SearchFinishedReceiver : public QObject
{
    Q_OBJECT

    private slots:
        void searchFinished(int id, Okular::Document::SearchStatus status)
        {
            m_id = id;
            m_status = status;
        }
    
    public:
        int m_id;
        Okular::Document::SearchStatus m_status;
};
    
class SearchTest : public QObject
{
    Q_OBJECT

    private slots:
        void initTestCase();
        void testNextAndPrevious();
        void test311232();
        void test323262();
        void test323263();
        void testDottedI();
        void testHyphenAtEndOfLineWithoutYOverlap();
        void testHyphenWithYOverlap();
        void testHyphenAtEndOfPage();
        void testOneColumn();
        void testTwoColumns();
};

void SearchTest::initTestCase()
{
    qRegisterMetaType<Okular::Document::SearchStatus>();
    Okular::SettingsCore::instance( "searchtest" );
}

static void createTextPage(const QVector<QString>& text, const QVector<Okular::NormalizedRect>& rect,
                                        Okular::TextPage*& tp, Okular::Page*& page) {

    tp = new Okular::TextPage();
    for (int i = 0; i < text.size(); i++) {
        tp->append(text[i], new Okular::NormalizedRect(rect[i]));
    }

    //The Page::setTextPage method invokes the layout analysis algorithms tested by some tests here
    //and also sets the tp->d->m_page field (the latter was used in older versions of Okular by
    //TextPage::stringLengthAdaptedWithHyphen).
    //Note that calling "delete page;" will delete the TextPage as well.
    page = new Okular::Page(1, 100, 100, Okular::Rotation0);
    page -> setTextPage(tp);
}

#define CREATE_PAGE \
  QCOMPARE(text.size(), rect.size()); \
  Okular::Page* page; \
  Okular::TextPage* tp; \
  createTextPage(text, rect, tp, page);

#define TEST_NEXT_PREV(searchType, expectedStatus) \
{ \
   Okular::RegularAreaRect* result = tp->findText(0, searchString, searchType, Qt::CaseSensitive, NULL); \
   QCOMPARE(!!result, expectedStatus); \
   delete result; \
}

//The test testNextAndPrevious checks that
//a) if one starts a new search, then the first or last match is found, depending on the search direction
//   (2 cases: FromTop/FromBottom)
//b) if the last search has found a match,
//   then clicking the "Next" button moves to the next occurrence an "Previous" to the previous one
//   (if there is any). Altogether there are four combinations of the last search and new search
//   direction: Next-Next, Previous-Previous, Next-Previous, Previous-Next; the first two combination
//   have two subcases (the new search may give a match or not, so altogether 6 cases to test).
//This gives 8 cases altogether. By taking into account the cases where the last search has given no match,
//we would have 4 more cases (Next (no match)-Next, Next (no match)-Previous, Previous (no match)-Previous,
//Previous (no match)-Next), but those are more the business of Okular::Document::searchText rather than
//Okular::TextPage (at least in the multi-page case).

//   We have four test situations: four documents and four corresponding search strings.
//   The first situation (document="ababa", search string="b") is a generic one where the
//two matches are not side-by-side and neither the first character nor the last character of
//the document match. The only special thing is that the search string has only length 1.
//   The second situation (document="abab", search string="ab") is notable for that the two occurrences
//of the search string are side-by-side with no characters in between, so some off-by-one errors
//would be detected by this test. As the first match starts at the beginning at the document the
//last match ends at the end of the document, it also detects off-by-one errors for finding the first/last match.
//   The third situation (document="abababa", search string="aba") is notable for it shows whether
//the next match is allowed to contain letters from the previous one: currently it is not
//(as in the majority of browsers, viewers and editors), and therefore "abababa" is considered to
//contain not three but two occurrences of "aba" (if one starts search from the beginning of the document).
//   The fourth situation (document="a ba b", search string="a b") demonstrates the case when one TinyTextEntity
//contains multiple characters that are contained in different matches (namely, the middle "ba" is one TinyTextEntity);
//in particular, since these matches are side-by-side, this test would detect some off-by-one
//offset errors.

void SearchTest::testNextAndPrevious()
{

#define TEST_NEXT_PREV_SITUATION_COUNT 4

    QVector<QString> texts[TEST_NEXT_PREV_SITUATION_COUNT] = {
        QVector<QString>() << "a" << "b" << "a" << "b" << "a",
        QVector<QString>() << "a" << "b" << "a" << "b",
        QVector<QString>() << "a" << "b" << "a" << "b" << "a" << "b" << "a" ,
        QVector<QString>() << "a" << " " << "ba" << " " << "b"
    };

    QString searchStrings[TEST_NEXT_PREV_SITUATION_COUNT] = {
        "b",
        "ab",
        "aba",
        "a b"
    };

    for (int i = 0; i < TEST_NEXT_PREV_SITUATION_COUNT; i++) {
        const QVector<QString>& text = texts[i];
        const QString& searchString = searchStrings[i];

        QVector<Okular::NormalizedRect> rect; \

        for (int i = 0; i < text.size(); i++) {
            rect << Okular::NormalizedRect(0.1*i, 0.0, 0.1*(i+1), 0.1); \
        }

        CREATE_PAGE;

        //Test 3 of the 8 cases listed above:
        //FromTop, Next-Next (match) and Next-Next (no match)
        TEST_NEXT_PREV(Okular::FromTop, true);
        TEST_NEXT_PREV(Okular::NextResult, true);
        TEST_NEXT_PREV(Okular::NextResult, false);

        //Test 5 cases: FromBottom, Previous-Previous (match), Previous-Next,
        //Next-Previous, Previous-Previous (no match)
        TEST_NEXT_PREV(Okular::FromBottom, true);
        TEST_NEXT_PREV(Okular::PreviousResult, true);
        TEST_NEXT_PREV(Okular::NextResult, true);
        TEST_NEXT_PREV(Okular::PreviousResult, true);
        TEST_NEXT_PREV(Okular::PreviousResult, false);

        delete page;
    }
}

void SearchTest::test311232()
{
    Okular::Document d(0);
    SearchFinishedReceiver receiver;
    QSignalSpy spy(&d, SIGNAL(searchFinished(int,Okular::Document::SearchStatus)));

    QObject::connect(&d, SIGNAL(searchFinished(int,Okular::Document::SearchStatus)), &receiver, SLOT(searchFinished(int,Okular::Document::SearchStatus)));
    
    const QString testFile = KDESRCDIR "data/file1.pdf";
    const KMimeType::Ptr mime = KMimeType::findByPath( testFile );
    d.openDocument(testFile, KUrl(), mime);
    
    const int searchId = 0;
    d.searchText(searchId, " i ", true, Qt::CaseSensitive, Okular::Document::NextMatch, false, QColor());
    QTime t;
    t.start();
    while (spy.count() != 1 && t.elapsed() < 500)
        qApp->processEvents();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(receiver.m_id, searchId);
    QCOMPARE(receiver.m_status, Okular::Document::MatchFound);
    
    
    d.continueSearch( searchId, Okular::Document::PreviousMatch );
    t.start();
    while (spy.count() != 2 && t.elapsed() < 500)
        qApp->processEvents();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(receiver.m_id, searchId);
    QCOMPARE(receiver.m_status, Okular::Document::NoMatchFound);
}

void SearchTest::test323262()
{
    QVector<QString> text;
    text << "a\n";

    QVector<Okular::NormalizedRect> rect;
    rect << Okular::NormalizedRect(1, 2, 3, 4);

    CREATE_PAGE;

    Okular::RegularAreaRect* result = tp->findText(0, "a", Okular::FromBottom, Qt::CaseSensitive, NULL);
    QVERIFY(result);
    delete result;

    delete page;
}

void SearchTest::test323263()
{
    QVector<QString> text;
    text << "a" << "a" << "b";

    QVector<Okular::NormalizedRect> rect;
    rect << Okular::NormalizedRect(0, 0, 1, 1)
         << Okular::NormalizedRect(1, 0, 2, 1)
         << Okular::NormalizedRect(2, 0, 3, 1);

    CREATE_PAGE;

    Okular::RegularAreaRect* result = tp->findText(0, "ab", Okular::FromTop, Qt::CaseSensitive, NULL);
    QVERIFY(result);
    Okular::RegularAreaRect expected;
    expected.append(rect[1]);
    expected.append(rect[2]);
    expected.simplify();
    QCOMPARE(*result, expected);
    delete result;

    delete page;
}

void SearchTest::testDottedI()
{
    //Earlier versions of okular had the bug that the letter "İ" (capital dotter i) did not match itself
    //in case-insensitive mode (this was caused by an unnecessary call of toLower() and the fact that
    //QString::fromUtf8("İ").compare(QString::fromUtf8("İ").toLower(), Qt::CaseInsensitive) == FALSE,
    //at least in Qt 4.8).

    //In the future it would be nice to add support for matching "İ"<->"i" and "I"<->"ı" in case-insensitive
    //mode as well (QString::compare does not match them, at least in non-Turkish locales, since it follows
    //the Unicode case-folding rules http://www.unicode.org/Public/6.2.0/ucd/CaseFolding.txt).

    QVector<QString> text;
    text << QString::fromUtf8("İ");

    QVector<Okular::NormalizedRect> rect;
    rect << Okular::NormalizedRect(1, 2, 3, 4);

    CREATE_PAGE;

    Okular::RegularAreaRect* result = tp->findText(0, QString::fromUtf8("İ"), Okular::FromTop, Qt::CaseInsensitive, NULL);
    QVERIFY(result);
    delete result;

    delete page;
}

void SearchTest::testHyphenAtEndOfLineWithoutYOverlap()
{
    QVector<QString> text;
    text << "super-"
         << "cali-\n"
         << "fragilistic" << "-"
         << "expiali" << "-\n"
         << "docious";

    QVector<Okular::NormalizedRect> rect;
    rect << Okular::NormalizedRect(0.4, 0.0, 0.9, 0.1)
         << Okular::NormalizedRect(0.0, 0.1, 0.6, 0.2)
         << Okular::NormalizedRect(0.0, 0.2, 0.8, 0.3) << Okular::NormalizedRect(0.8, 0.2, 0.9, 0.3)
         << Okular::NormalizedRect(0.0, 0.3, 0.8, 0.4) << Okular::NormalizedRect(0.8, 0.3, 0.9, 0.4)
         << Okular::NormalizedRect(0.0, 0.4, 0.7, 0.5);

    CREATE_PAGE;

    Okular::RegularAreaRect* result = tp->findText(0, "supercalifragilisticexpialidocious",
                                                 Okular::FromTop, Qt::CaseSensitive, NULL);
    QVERIFY(result);
    Okular::RegularAreaRect expected;
    for (int i = 0; i < text.size(); i++) {
        expected.append(rect[i]);
    }
    expected.simplify();
    QCOMPARE(*result, expected);
    delete result;

    delete page;
}

#define CREATE_PAGE_AND_TEST_SEARCH(searchString, matchExpected) \
{ \
    CREATE_PAGE; \
 \
    Okular::RegularAreaRect* result = tp->findText(0, searchString, \
               Okular::FromTop, Qt::CaseSensitive, NULL); \
 \
    QCOMPARE(!!result, matchExpected); \
 \
    delete result; \
    delete page; \
}

void SearchTest::testHyphenWithYOverlap()
{
    QVector<QString> text;
    text << "a-"
         << "b";

    QVector<Okular::NormalizedRect> rect(2);

    //different lines (50% y-coordinate overlap), first rectangle has larger height
    rect[0] = Okular::NormalizedRect(0.0, 0.0,  0.9, 0.35);
    rect[1] = Okular::NormalizedRect(0.0, 0.3, 0.2, 0.4);
    CREATE_PAGE_AND_TEST_SEARCH("ab", true);

    //different lines (50% y-coordinate overlap), second rectangle has larger height
    rect[0] = Okular::NormalizedRect(0.0, 0.0,  0.9, 0.1);
    rect[1] = Okular::NormalizedRect(0.0, 0.05, 0.2, 0.4);
    CREATE_PAGE_AND_TEST_SEARCH("ab", true);

    //same line (90% y-coordinate overlap), first rectangle has larger height
    rect[0] = Okular::NormalizedRect(0.0, 0.0,  0.4, 0.2);
    rect[1] = Okular::NormalizedRect(0.4, 0.11, 0.6, 0.21);
    CREATE_PAGE_AND_TEST_SEARCH("ab", false);
    CREATE_PAGE_AND_TEST_SEARCH("a-b", true);

    //same line (90% y-coordinate overlap), second rectangle has larger height
    rect[0] = Okular::NormalizedRect(0.0, 0.0, 0.4, 0.1);
    rect[1] = Okular::NormalizedRect(0.4, 0.01, 0.6, 0.2);
    CREATE_PAGE_AND_TEST_SEARCH("ab", false);
    CREATE_PAGE_AND_TEST_SEARCH("a-b", true);
}

void SearchTest::testHyphenAtEndOfPage()
{
    //Tests for segmentation fault that would occur if
    //we tried look ahead (for determining whether the
    //next character is at the same line) at the end of the page.

    QVector<QString> text;
    text << "a-";

    QVector<Okular::NormalizedRect> rect;
    rect << Okular::NormalizedRect(0, 0, 1, 1);

    CREATE_PAGE;

    {
        Okular::RegularAreaRect* result = tp->findText(0, "a",
            Okular::FromTop, Qt::CaseSensitive, NULL);
        QVERIFY(result);
        delete result;
    }

    {
        Okular::RegularAreaRect* result = tp->findText(0, "a",
            Okular::FromBottom, Qt::CaseSensitive, NULL);
        QVERIFY(result);
        delete result;
    }

    delete page;
}

void SearchTest::testOneColumn()
{
  //Tests that the layout analysis algorithm does not create too many columns.
  //Bug 326207 was caused by the fact that if all the horizontal breaks in a line
  //had the same length and were smaller than vertical breaks between lines then
  //the horizontal breaks were treated as column separators.
  //(Note that "same length" means "same length after rounding rectangles to integer pixels".
  //The resolution used by the XY Cut algorithm with a square page is 1000 x 1000,
  //and the horizontal spaces in the example are 0.1, so they are indeed both exactly 100 pixels.)

  QVector<QString> text;
  text << "Only" << "one" << "column"
       << "here";

  //characters and line breaks have length 0.05, word breaks 0.1
  QVector<Okular::NormalizedRect> rect;
  rect << Okular::NormalizedRect(0.0,  0.0,  0.2,  0.1)
       << Okular::NormalizedRect(0.3,  0.0,  0.5,  0.1)
       << Okular::NormalizedRect(0.6,  0.0,  0.9,  0.1)
       << Okular::NormalizedRect(0.0,  0.15, 0.2,  0.25);

  CREATE_PAGE;

  Okular::RegularAreaRect* result = tp->findText(0, "Only one column",
      Okular::FromTop, Qt::CaseSensitive, NULL);
  QVERIFY(result);
  delete result;

  delete page;
}

void SearchTest::testTwoColumns()
{
  //Tests that the layout analysis algorithm can detect two columns.

  QVector<QString> text;
  text << "This" << "text" << "in" << "two"
       << "is" << "set"    << "columns.";

  //characters, word breaks and line breaks have length 0.05
  QVector<Okular::NormalizedRect> rect;
  rect << Okular::NormalizedRect(0.0,  0.0,  0.20, 0.1)
       << Okular::NormalizedRect(0.25, 0.0,  0.45, 0.1)
       << Okular::NormalizedRect(0.6,  0.0,  0.7,  0.1)
       << Okular::NormalizedRect(0.75, 0.0,  0.9,  0.1)
       << Okular::NormalizedRect(0.0,  0.15, 0.1,  0.25)
       << Okular::NormalizedRect(0.15, 0.15, 0.3,  0.25)
       << Okular::NormalizedRect(0.6,  0.15, 1.0,  0.25);

  CREATE_PAGE;

  Okular::RegularAreaRect* result = tp->findText(0, "This text in",
      Okular::FromTop, Qt::CaseSensitive, NULL);
  QVERIFY(!result);
  delete result;

  delete page;
}

QTEST_KDEMAIN( SearchTest, GUI )

#include "searchtest.moc"
