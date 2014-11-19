/***************************************************************************
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_ACTION_H_
#define _OKULAR_ACTION_H_

#include "global.h"
#include "okular_export.h"

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace Okular {

class ActionPrivate;
class GotoActionPrivate;
class ExecuteActionPrivate;
class BrowseActionPrivate;
class DocumentActionPrivate;
class SoundActionPrivate;
class ScriptActionPrivate;
class MovieActionPrivate;
class RenditionActionPrivate;
class MovieAnnotation;
class ScreenAnnotation;
class Movie;
class Sound;
class DocumentViewport;

/**
 * @short Encapsulates data that describes an action.
 *
 * This is the base class for actions. It makes mandatory for inherited
 * widgets to reimplement the 'actionType' method and return the type of
 * the action described by the reimplemented class.
 */
class OKULAR_EXPORT Action
{
    public:
        /**
         * Describes the type of action.
         */
        enum ActionType {
            Goto,       ///< Goto a given page or external document
            Execute,    ///< Execute a command or external application
            Browse,     ///< Browse a given website
            DocAction,  ///< Start a custom action
            Sound,      ///< Play a sound
            Movie,      ///< Play a movie
            Script,     ///< Executes a Script code
            Rendition   ///< Play a movie and/or execute a Script code @since 0.16 (KDE 4.10)
        };

        /**
         * Destroys the action.
         */
        virtual ~Action();

        /**
         * Returns the type of the action. Every inherited class must return
         * an unique identifier.
         *
         * @see ActionType
         */
        virtual ActionType actionType() const = 0;

        /**
         * Returns a i18n'ed tip of the action that is presented to
         * the user.
         */
        virtual QString actionTip() const;

        /**
         * Sets the "native" @p id of the action.
         *
         * This is for use of the Generator, that can optionally store an
         * handle (a pointer, an identifier, etc) of the "native" action
         * object, if any.
         *
         * @note Okular makes no use of this
         *
         * @since 0.15 (KDE 4.9)
         */
        void setNativeId( const QVariant &id );

        /**
         * Returns the "native" id of the action.
         *
         * @since 0.15 (KDE 4.9)
         */
        QVariant nativeId() const;

    protected:
        /// @cond PRIVATE
        Action( ActionPrivate &dd );
        Q_DECLARE_PRIVATE( Action )
        ActionPrivate *d_ptr;
        /// @endcond

    private:
        Q_DISABLE_COPY( Action )
};


/**
 * The Goto action changes the viewport to another page
 * or loads an external document.
 */
class OKULAR_EXPORT GotoAction : public Action
{
    public:
        /**
         * Creates a new goto action.
         *
         * @p fileName The name of an external file that shall be loaded.
         * @p viewport The target viewport information of the current document.
         */
        GotoAction( const QString& fileName, const DocumentViewport & viewport );

        /**
         * Creates a new goto action.
         *
         * @p fileName The name of an external file that shall be loaded.
         * @p namedDestination The target named destination for the target document.
         *
         * @since 0.9 (KDE 4.3)
         */
        GotoAction( const QString& fileName, const QString& namedDestination );

        /**
         * Destroys the goto action.
         */
        virtual ~GotoAction();

        /**
         * Returns the action type.
         */
        ActionType actionType() const;

        /**
         * Returns the action tip.
         */
        QString actionTip() const;

        /**
         * Returns whether the goto action points to an external document.
         */
        bool isExternal() const;

        /**
         * Returns the filename of the external document.
         */
        QString fileName() const;

        /**
         * Returns the document viewport the goto action points to.
         */
        DocumentViewport destViewport() const;

        /**
         * Returns the document named destination the goto action points to.
         *
         * @since 0.9 (KDE 4.3)
         */
        QString destinationName() const;

    private:
        Q_DECLARE_PRIVATE( GotoAction )
        Q_DISABLE_COPY( GotoAction )
};

/**
 * The Execute action executes an external application.
 */
class OKULAR_EXPORT ExecuteAction : public Action
{
    public:
        /**
         * Creates a new execute action.
         *
         * @param fileName The file name of the application to execute.
         * @param parameters The parameters of the application to execute.
         */
        ExecuteAction( const QString &fileName, const QString &parameters );

        /**
         * Destroys the execute action.
         */
        virtual ~ExecuteAction();

        /**
         * Returns the action type.
         */
        ActionType actionType() const;

        /**
         * Returns the action tip.
         */
        QString actionTip() const;

        /**
         * Returns the file name of the application to execute.
         */
        QString fileName() const;

        /**
         * Returns the parameters of the application to execute.
         */
        QString parameters() const;

    private:
        Q_DECLARE_PRIVATE( ExecuteAction )
        Q_DISABLE_COPY( ExecuteAction )
};

/**
 * The Browse action browses an url by opening a web browser or
 * email client, depedning on the url protocol (e.g. http, mailto, etc.).
 */
class OKULAR_EXPORT BrowseAction : public Action
{
    public:
        /**
         * Creates a new browse action.
         *
         * @param url The url to browse.
         */
        BrowseAction( const QString &url );

        /**
         * Destroys the browse action.
         */
        virtual ~BrowseAction();

        /**
         * Returns the action type.
         */
        ActionType actionType() const;

        /**
         * Returns the action tip.
         */
        QString actionTip() const;

        /**
         * Returns the url to browse.
         */
        QString url() const;

    private:
        Q_DECLARE_PRIVATE( BrowseAction )
        Q_DISABLE_COPY( BrowseAction )
};

/**
 * The DocumentAction action contains an action that is performed on
 * the current document.
 */
class OKULAR_EXPORT DocumentAction : public Action
{
    public:
        /**
         * Describes the possible action types.
         *
         * WARNING KEEP IN SYNC WITH POPPLER!
         */
        enum DocumentActionType {
            PageFirst = 1,        ///< Jump to first page
            PagePrev = 2,         ///< Jump to previous page
            PageNext = 3,         ///< Jump to next page
            PageLast = 4,         ///< Jump to last page
            HistoryBack = 5,      ///< Go back in page history
            HistoryForward = 6,   ///< Go forward in page history
            Quit = 7,             ///< Quit application
            Presentation = 8,     ///< Start presentation
            EndPresentation = 9,  ///< End presentation
            Find = 10,            ///< Open find dialog
            GoToPage = 11,        ///< Goto page
            Close = 12            ///< Close document
        };

        /**
         * Creates a new document action.
         *
         * @param documentActionType The type of document action.
         */
        explicit DocumentAction( enum DocumentActionType documentActionType );

        /**
         * Destroys the document action.
         */
        virtual ~DocumentAction();

        /**
         * Returns the action type.
         */
        ActionType actionType() const;

        /**
         * Returns the action tip.
         */
        QString actionTip() const;

        /**
         * Returns the type of action.
         */
        DocumentActionType documentActionType() const;

    private:
        Q_DECLARE_PRIVATE( DocumentAction )
        Q_DISABLE_COPY( DocumentAction )
};

/**
 * The Sound action plays a sound on activation.
 */
class OKULAR_EXPORT SoundAction : public Action
{
    public:
        /**
         * Creates a new sound action.
         *
         * @param volume The volume of the sound.
         * @param synchronous Whether the sound shall be played synchronous.
         * @param repeat Whether the sound shall be repeated.
         * @param mix Whether the sound shall be mixed.
         * @param sound The sound object which contains the sound data.
         */
        SoundAction( double volume, bool synchronous, bool repeat, bool mix, Okular::Sound *sound );

        /**
         * Destroys the sound action.
         */
        virtual ~SoundAction();

        /**
         * Returns the action type.
         */
        ActionType actionType() const;

        /**
         * Returns the action tip.
         */
        QString actionTip() const;

        /**
         * Returns the volume of the sound.
         */
        double volume() const;

        /**
         * Returns whether the sound shall be played synchronous.
         */
        bool synchronous() const;

        /**
         * Returns whether the sound shall be repeated.
         */
        bool repeat() const;

        /**
         * Returns whether the sound shall be mixed.
         */
        bool mix() const;

        /**
         * Returns the sound object which contains the sound data.
         */
        Okular::Sound *sound() const;

    private:
        Q_DECLARE_PRIVATE( SoundAction )
        Q_DISABLE_COPY( SoundAction )
};

/**
 * The Script action executes a Script code.
 *
 * @since 0.7 (KDE 4.1)
 */
class OKULAR_EXPORT ScriptAction : public Action
{
    public:
        /**
         * Creates a new Script action.
         *
         * @param script The code to execute.
         */
        ScriptAction( enum ScriptType type, const QString &script );

        /**
         * Destroys the browse action.
         */
        virtual ~ScriptAction();

        /**
         * Returns the action type.
         */
        ActionType actionType() const;

        /**
         * Returns the action tip.
         */
        QString actionTip() const;

        /**
         * Returns the type of action.
         */
        ScriptType scriptType() const;

        /**
         * Returns the code.
         */
        QString script() const;

    private:
        Q_DECLARE_PRIVATE( ScriptAction )
        Q_DISABLE_COPY( ScriptAction )
};

/**
 * The Movie action executes an operation on a video on activation.
 *
 * @since 0.15 (KDE 4.9)
 */
class OKULAR_EXPORT MovieAction : public Action
{
    public:
        /**
         * Describes the possible operation types.
         */
        enum OperationType {
            Play,
            Stop,
            Pause,
            Resume
        };

        /**
         * Creates a new movie action.
         */
        MovieAction( OperationType operation );

        /**
         * Destroys the movie action.
         */
        virtual ~MovieAction();

        /**
         * Returns the action type.
         */
        ActionType actionType() const;

        /**
         * Returns the action tip.
         */
        QString actionTip() const;

        /**
         * Returns the operation type.
         */
        OperationType operation() const;

        /**
         * Sets the @p annotation that is associated with the movie action.
         */
        void setAnnotation( MovieAnnotation *annotation );

        /**
         * Returns the annotation or @c 0 if no annotation has been set.
         */
        MovieAnnotation* annotation() const;

    private:
        Q_DECLARE_PRIVATE( MovieAction )
        Q_DISABLE_COPY( MovieAction )
};

/**
 * The Rendition action executes an operation on a video or
 * executes some JavaScript code on activation.
 *
 * @since 0.16 (KDE 4.10)
 */
class OKULAR_EXPORT RenditionAction : public Action
{
    public:
        /**
         * Describes the possible operation types.
         */
        enum OperationType {
            None,   ///< Execute only the JavaScript
            Play,   ///< Start playing the video
            Stop,   ///< Stop playing the video
            Pause,  ///< Pause the video
            Resume  ///< Resume playing the video
        };

        /**
         * Creates a new rendition action.
         *
         * @param operation The type of operation the action executes.
         * @param movie The movie object the action references.
         * @param scriptType The type of script the action executes.
         * @param script The actual script the action executes.
         */
        RenditionAction( OperationType operation, Okular::Movie *movie, enum ScriptType scriptType, const QString &script );

        /**
         * Destroys the rendition action.
         */
        virtual ~RenditionAction();

        /**
         * Returns the action type.
         */
        ActionType actionType() const;

        /**
         * Returns the action tip.
         */
        QString actionTip() const;

        /**
         * Returns the operation type.
         */
        OperationType operation() const;

        /**
         * Returns the movie object or @c 0 if no movie object was set on construction time.
         */
        Okular::Movie* movie() const;

        /**
         * Returns the type of script.
         */
        ScriptType scriptType() const;

        /**
         * Returns the script code.
         */
        QString script() const;

        /**
         * Sets the @p annotation that is associated with the rendition action.
         */
        void setAnnotation( ScreenAnnotation *annotation );

        /**
         * Returns the annotation or @c 0 if no annotation has been set.
         */
        ScreenAnnotation* annotation() const;

    private:
        Q_DECLARE_PRIVATE( RenditionAction )
        Q_DISABLE_COPY( RenditionAction )
};

}

#endif
