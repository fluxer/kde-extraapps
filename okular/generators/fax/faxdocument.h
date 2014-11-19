/***************************************************************************
 *   Copyright (C) 2008 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef FAXDOCUMENT_H
#define FAXDOCUMENT_H

#include <QtGui/QImage>

/**
 * Loads a G3/G4 fax document and provides methods
 * to convert it into a QImage.
 */
class FaxDocument
{
  public:
    /**
     * Describes the type of the fax document.
     */
    enum DocumentType
    {
      G3,  ///< G3 encoded fax document
      G4   ///< G4 encoded fax document
    };

    /**
     * Creates a new fax document from the given @p fileName.
     *
     * @param type The type of the fax document.
     */
    explicit FaxDocument( const QString &fileName, DocumentType type = G3 );

    /**
     * Destroys the fax document.
     */
    ~FaxDocument();

    /**
     * Loads the document.
     *
     * @return @c true if the document can be loaded successfully, @c false otherwise.
     */
    bool load();

    /**
     * Returns the document as an image.
     */
    QImage image() const;

  private:
    class Private;
    Private* const d;

    FaxDocument( const FaxDocument& );
    FaxDocument& operator=( const FaxDocument& );
};

#endif
