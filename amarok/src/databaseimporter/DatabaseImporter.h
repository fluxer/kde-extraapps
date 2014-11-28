/****************************************************************************************
 * Copyright (c) 2008 Seb Ruiz <ruiz@kde.org>                                           *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef AMAROK_DATABASE_IMPORTER_H
#define AMAROK_DATABASE_IMPORTER_H

#include "core/meta/forward_declarations.h"

#include <KVBox>

class DatabaseImporter;

class DatabaseImporterFactory
{
    public:
        static DatabaseImporter* createImporter( const QString &name, QObject *parent );
};

class DatabaseImporterConfig : public KVBox
{
    public:
        DatabaseImporterConfig( QWidget *parent = 0 ) : KVBox( parent ) { }
        virtual ~DatabaseImporterConfig() { };
};

class DatabaseImporter : public QObject
{
    Q_OBJECT

    public:
        DatabaseImporter( QObject *parent=0 );
        virtual ~DatabaseImporter();

        virtual DatabaseImporterConfig *configWidget( QWidget *parent );

        /**
         * @return whether the importer is running
         */
        virtual bool importing() const;

        /**
         * Starts the importing process
         */
        virtual void startImporting();

        /**
         * @returns the number of tracks imported
         */
        int importedCount() const;

    signals:
        void importFailed();
        void importSucceeded();
        void importError( QString );
        void showMessage( QString );
        void trackAdded( Meta::TrackPtr );
        void trackDiscarded( QString );
        void trackMatchFound( Meta::TrackPtr, QString );
        void trackMatchMultiple( Meta::TrackList, QString );

    protected slots:
        void importingFinished();
        void trackImported( Meta::TrackPtr track );
        void trackMatched( Meta::TrackPtr track, QString oldUrl );

    protected:
        /**
         * The internal implementation of the importing algorithm.
         */
        virtual void import() = 0;

        bool m_importing;

    private:
        int m_count;
};

#endif // multiple inclusion guard
