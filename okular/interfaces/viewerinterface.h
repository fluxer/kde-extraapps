/***************************************************************************
 *   Copyright (C) 2011 by Michel Ludwig <michel.ludwig@kdemail.net>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_VIEWERINTERFACE_H_
#define _OKULAR_VIEWERINTERFACE_H_

#include "../core/okular_export.h"

#include <QtCore/QObject>
#include <QtCore/QString>

namespace Okular {

/**
 * @short Abstract interface for controlling advanced features of a document viewer
 *
 * This interface can be used to control some more or less advanced features of a document
 * viewer.
 */
class ViewerInterface
{
    public:
        virtual ~ViewerInterface() {}

        /**
         * Show the specified source location centrally in the viewer.
         *
         * @param fileName source file name
         * @param line in the source file, starts from 0
         * @param column in the source file, starts from 0
         * @param showGraphically controls whether the given source location will be
         *                        shown graphically in the viewer (if that feature is globally activated)
         */
        virtual void showSourceLocation(const QString& fileName, int line, int column, bool showGraphically = true) = 0;

        /**
         * Clear the source location that was set last in the viewer.
         */
        virtual void clearLastShownSourceLocation() = 0;

        /**
         * Returns true iff source locations are shown graphically.
         */
        virtual bool areSourceLocationsShownGraphically() const = 0;

        /**
         * Allows to control whether source locations are shown graphically, or not.
         */
        virtual void setShowSourceLocationsGraphically(bool b) = 0;

        /**
         * Returns true iff the watch file mode is enabled.
         */
        virtual bool isWatchFileModeEnabled() const = 0;

        /**
         * Allows to enable or disable the watch file mode
         */
        virtual void setWatchFileModeEnabled(bool b) = 0;

        /**
         * Should the shell that supports tabs pen new files in tabs?
         */
        virtual bool openNewFilesInTabs() const = 0;

        // SIGNALS
        /**
         * The signal 'openSourceReference' is emitted whenever the user has triggered a source
         * reference in the currently displayed document.
         */
        void openSourceReference(const QString& absFileName, int line, int column);

        /**
         * The signal 'viewerMenuStateChange' is emitted whenever the state of the menu
         * 'menu_okular_part_viewer' defined in 'part-viewermode.rc' has changed.
         */
        void viewerMenuStateChange(bool enabled);
};

}

Q_DECLARE_INTERFACE( Okular::ViewerInterface, "org.kde.okular.ViewerInterface/0.1" )

#endif
