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
#include "core/document.h"
#include <core/page.h>
#include <core/form.h>

class EditFormsTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testRadioButtonForm();
    void testCheckBoxForm();
    void testTextLineForm();
    void testTextAreaForm();
    void testFileEditForm();
    void testComboEditForm();
    void testListSingleEdit();
    void testListMultiEdit();

    // helper methods
    void verifyRadioButtonStates(bool state1, bool state2, bool state3);
    void setRadioButtonStates(bool state1, bool state2, bool state3);
    void verifyTextForm( Okular::FormFieldText *form );

private:
    Okular::Document *m_document;
    QList<Okular::FormFieldButton*> m_radioButtonForms;
    QList<Okular::FormFieldButton*> m_checkBoxForms;
    Okular::FormFieldText* m_textLineForm;
    Okular::FormFieldText* m_textAreaForm;
    Okular::FormFieldText* m_fileEditForm;
    Okular::FormFieldChoice* m_comboEdit;
    Okular::FormFieldChoice* m_listSingleEdit;
    Okular::FormFieldChoice* m_listMultiEdit;
};

void EditFormsTest::initTestCase()
{
    Okular::SettingsCore::instance( "editformstest" );
    m_document = new Okular::Document( 0 );
}

void EditFormsTest::cleanupTestCase()
{
    delete m_document;
}

void EditFormsTest::init()
{
    const QString testFile = KDESRCDIR "data/formSamples.pdf";
    const KMimeType::Ptr mime = KMimeType::findByPath( testFile );
    m_document->openDocument( testFile, KUrl(), mime );

    // Undo and Redo should be unavailable when docuemnt is first opened.
    QVERIFY( !m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    const Okular::Page* page = m_document->page( 0 );
    QLinkedList<Okular::FormField*> pageFields = page->formFields();

    // Clear lists
    m_checkBoxForms.clear();
    m_radioButtonForms.clear();

    // Collect forms of the various types
    QLinkedList< Okular::FormField * >::const_iterator ffIt = pageFields.constBegin(), ffEnd = pageFields.constEnd();
    for ( ; ffIt != ffEnd; ++ffIt )
    {
        Okular::FormField * ff = *ffIt;
        ff->type();

        switch ( ff->type() )
        {
            case Okular::FormField::FormButton:
            {
                Okular::FormFieldButton * ffb = static_cast< Okular::FormFieldButton * >( ff );
                switch ( ffb->buttonType() )
                {
                    case Okular::FormFieldButton::Push:
                        break;
                    case Okular::FormFieldButton::CheckBox:
                        m_checkBoxForms.append(ffb);
                        break;
                    case Okular::FormFieldButton::Radio:
                        m_radioButtonForms.append(ffb);
                        break;
                    default: ;
                }
                break;
            }
            case Okular::FormField::FormText:
            {
                Okular::FormFieldText * fft = static_cast< Okular::FormFieldText * >( ff );
                switch ( fft->textType() )
                {
                    case Okular::FormFieldText::Multiline:
                        m_textAreaForm = fft;
                        break;
                    case Okular::FormFieldText::Normal:
                        m_textLineForm = fft;
                        break;
                    case Okular::FormFieldText::FileSelect:
                        m_fileEditForm = fft;
                        break;
                }
                break;
            }
            case Okular::FormField::FormChoice:
            {
                Okular::FormFieldChoice * ffc = static_cast< Okular::FormFieldChoice * >( ff );
                switch ( ffc->choiceType() )
                {
                    case Okular::FormFieldChoice::ListBox:
                        if ( ffc->multiSelect() )
                        {
                            m_listMultiEdit = ffc;
                        }
                        else
                        {
                            m_listSingleEdit = ffc;
                        }
                        break;
                    case Okular::FormFieldChoice::ComboBox:
                        m_comboEdit = ffc;
                        break;
                }
                break;
            }
            default: ;
        }
    }
}

void EditFormsTest::cleanup()
{
    m_document->closeDocument();
}

void EditFormsTest::testRadioButtonForm()
{
    // Initially the first radio button is checked
    verifyRadioButtonStates( true, false, false );

    // Set the second radio to checked and make sure the first
    // is now unchecked and that an undo action is available
    setRadioButtonStates( false, true, false);
    verifyRadioButtonStates( false, true, false );
    QVERIFY( m_document->canUndo() );

    // Now undo the action
    m_document->undo();
    verifyRadioButtonStates( true, false, false );
    QVERIFY( !m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Now redo the action
    m_document->redo();
    verifyRadioButtonStates( false, true, false );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );
}

void EditFormsTest::testCheckBoxForm()
{
    // Examine the first and second checkboxes
    // Initially both checkboxes are unchecked
    QVERIFY ( m_checkBoxForms[0]->state() == false );
    QVERIFY ( m_checkBoxForms[1]->state() == false );

    // Set checkbox 1 to true
    m_document->editFormButtons( 0, QList<Okular::FormFieldButton*>() << m_checkBoxForms[0], QList<bool>() << true);
    QVERIFY ( m_checkBoxForms[0]->state() == true );
    QVERIFY ( m_checkBoxForms[1]->state() == false );
    QVERIFY( m_document->canUndo() );

    // Set checkbox 2 to true
    m_document->editFormButtons( 0, QList<Okular::FormFieldButton*>() << m_checkBoxForms[1], QList<bool>() << true);
    QVERIFY ( m_checkBoxForms[0]->state() == true );
    QVERIFY ( m_checkBoxForms[1]->state() == true );
    QVERIFY( m_document->canUndo() );

    // Undo checking of second checkbox
    m_document->undo();
    QVERIFY ( m_checkBoxForms[0]->state() == true );
    QVERIFY ( m_checkBoxForms[1]->state() == false );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Undo checking of first checkbox
    m_document->undo();
    QVERIFY ( m_checkBoxForms[0]->state() == false );
    QVERIFY ( m_checkBoxForms[1]->state() == false );
    QVERIFY( !m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Redo checking of first checkbox
    m_document->redo();
    QVERIFY ( m_checkBoxForms[0]->state() == true );
    QVERIFY ( m_checkBoxForms[1]->state() == false );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );
}

void EditFormsTest::testTextLineForm()
{
    verifyTextForm( m_textLineForm );
}

void EditFormsTest::testTextAreaForm()
{
    verifyTextForm( m_textAreaForm );
}

void EditFormsTest::testFileEditForm()
{
    verifyTextForm( m_fileEditForm );
}

void EditFormsTest::testComboEditForm()
{
    // Editable combo with predefined choices:
    //     - combo1
    //     - combo2
    //     - combo3

    // Initially no choice is selected
    QCOMPARE( m_comboEdit->currentChoices().length(), 0 );
    QCOMPARE( m_comboEdit->editChoice(), QString( "" ) );

    // Select first choice
    m_document->editFormCombo( 0, m_comboEdit, "combo1", 0, 0, 0);
    QCOMPARE( m_comboEdit->currentChoices().length(), 1 );
    QCOMPARE( m_comboEdit->currentChoices()[0], 0 );
    QCOMPARE( m_comboEdit->editChoice(), QString( "" ) );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Select third choice
    m_document->editFormCombo( 0, m_comboEdit, "combo3", 0, 0, 0);
    QCOMPARE( m_comboEdit->currentChoices().length(), 1 );
    QCOMPARE( m_comboEdit->currentChoices()[0], 2 );
    QCOMPARE( m_comboEdit->editChoice(), QString( "" ) );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Undo and verify that first choice is selected
    m_document->undo();
    QCOMPARE( m_comboEdit->currentChoices().length(), 1 );
    QCOMPARE( m_comboEdit->currentChoices()[0], 0 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Redo and verify that third choice is selected
    m_document->redo();
    QCOMPARE( m_comboEdit->currentChoices().length(), 1 );
    QCOMPARE( m_comboEdit->currentChoices()[0], 2 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Select a custom choice and verify that no predefined choices are selected
    m_document->editFormCombo( 0, m_comboEdit, "comboEdit", 0, 0, 0);
    QCOMPARE( m_comboEdit->currentChoices().length(), 0 );
    QCOMPARE( m_comboEdit->editChoice(), QString( "comboEdit" ) );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Undo and verify that third choice is selected
    m_document->undo();
    QCOMPARE( m_comboEdit->currentChoices().length(), 1 );
    QCOMPARE( m_comboEdit->currentChoices()[0], 2 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );
}

void EditFormsTest::testListSingleEdit()
{
    // A list with three items that allows only single selections
    // Initially no choice is selected
    QCOMPARE( m_listSingleEdit->currentChoices().length(), 0 );

    // Select first item
    m_document->editFormList( 0, m_listSingleEdit, QList<int>() << 0);
    QCOMPARE( m_listSingleEdit->currentChoices().length(), 1 );
    QCOMPARE( m_listSingleEdit->currentChoices()[0], 0 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Select second item
    m_document->editFormList( 0, m_listSingleEdit, QList<int>() << 1);
    QCOMPARE( m_listSingleEdit->currentChoices().length(), 1 );
    QCOMPARE( m_listSingleEdit->currentChoices()[0], 1 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Undo and verify that first item is selected
    m_document->undo();
    QCOMPARE( m_listSingleEdit->currentChoices().length(), 1 );
    QCOMPARE( m_listSingleEdit->currentChoices()[0], 0 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Redo and verify that second item is selected
    m_document->redo();
    QCOMPARE( m_listSingleEdit->currentChoices().length(), 1 );
    QCOMPARE( m_listSingleEdit->currentChoices()[0], 1 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );
}

void EditFormsTest::testListMultiEdit()
{
    // A list with three items that allows for multiple selections
    // Initially no choice is selected
    QCOMPARE( m_listMultiEdit->currentChoices().length(), 0 );

    // Select first item
    m_document->editFormList( 0, m_listMultiEdit, QList<int>() << 0);
    QCOMPARE( m_listMultiEdit->currentChoices(), QList<int>() << 0 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Select first and third items
    m_document->editFormList( 0, m_listMultiEdit, QList<int>() << 0 << 2);
    QCOMPARE( m_listMultiEdit->currentChoices(), QList<int>() << 0 << 2 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Select all three items
    m_document->editFormList( 0, m_listMultiEdit, QList<int>() << 0 << 1 << 2);
    QCOMPARE( m_listMultiEdit->currentChoices(), QList<int>() << 0 << 1 << 2);
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Undo and verify that first and third items are selected
    m_document->undo();
    QCOMPARE( m_listMultiEdit->currentChoices(), QList<int>() << 0 << 2 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Undo and verify that first item is selected
    m_document->undo();
    QCOMPARE( m_listMultiEdit->currentChoices(), QList<int>() << 0 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Redo and verify that first and third items are selected
    m_document->redo();
    QCOMPARE( m_listMultiEdit->currentChoices(), QList<int>() << 0 << 2 );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );
}

// helper methods
void EditFormsTest::verifyRadioButtonStates( bool state1, bool state2, bool state3 )
{
    QVERIFY ( m_radioButtonForms[0]->state() == state1 );
    QVERIFY ( m_radioButtonForms[1]->state() == state2 );
    QVERIFY ( m_radioButtonForms[2]->state() == state3 );
}

void EditFormsTest::setRadioButtonStates( bool state1, bool state2, bool state3 )
{
    QList<bool> newButtonStates;
    newButtonStates.append(state1);
    newButtonStates.append(state2);
    newButtonStates.append(state3);
    m_document->editFormButtons( 0, m_radioButtonForms, newButtonStates);
}

void EditFormsTest::verifyTextForm( Okular::FormFieldText* form )
{
    // Text in form is initially empty
    QCOMPARE( form->text(), QString("") );

    // Insert the string "Hello" into the form
    m_document->editFormText(0, form, QString("Hello"), 5, 0, 0);
    QCOMPARE( form->text(), QString("Hello") );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Undo the insertion and verify that form is empty again
    m_document->undo();
    QCOMPARE( form->text(), QString("") );
    QVERIFY( !m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Redo the insertion of "Hello"
    m_document->redo();
    QCOMPARE( form->text(), QString("Hello") );
    QVERIFY( m_document->canUndo() );
    QVERIFY( !m_document->canRedo() );

    // Type "_World" after "Hello"
    m_document->editFormText(0, form, QString("Hello_"), 6, 5, 5);
    m_document->editFormText(0, form, QString("Hello_W"), 7, 6, 6);
    m_document->editFormText(0, form, QString("Hello_Wo"), 8, 7, 7);
    m_document->editFormText(0, form, QString("Hello_Wor"), 9, 8, 8);
    m_document->editFormText(0, form, QString("Hello_Worl"), 10, 9, 9);
    m_document->editFormText(0, form, QString("Hello_World"), 11, 10, 10);

    // Verify that character insertion operations were merged together into a single undo command
    m_document->undo();
    QCOMPARE( form->text(), QString("Hello") );
    QVERIFY( m_document->canUndo() );
    QVERIFY( m_document->canRedo() );

    // Verify that one more undo gets us back to the original state (empty form)
    m_document->undo();
    QCOMPARE( form->text(), QString("") );
    QVERIFY( !m_document->canUndo() );
    QVERIFY( m_document->canRedo() );
}

QTEST_KDEMAIN( EditFormsTest, GUI )
#include "editformstest.moc"
