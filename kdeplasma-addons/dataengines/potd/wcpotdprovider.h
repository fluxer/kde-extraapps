/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>                 
 *
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
 *   (at your option) any later version.   
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef WCPOTDPROVIDER_H
#define WCPOTDPROVIDER_H

#include "potdprovider.h"

/**
 * This class provides the image for the "Wikimedia 
 * Commons Picture Of the Day"
 * located at http://tools.wikimedia.de/~daniel/potd/commons/potd-800x600.html.
 * From there extract the picture.
 * Using 800x600 as the best size for now, others are available, see 
 * http://tools.wikimedia.de/~daniel/potd/potd.php
 */
class WcpotdProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new Wcpotd provider.
         *
         * @param date The date for which the image shall be fetched.
         * @param parent The parent object.
         */
        WcpotdProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the Wcpotd provider.
         */
        ~WcpotdProvider();

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        virtual QImage image() const;

    private:
      class Private;
      Private* const d;

      Q_PRIVATE_SLOT( d, void pageRequestFinished( KJob* ) )
      Q_PRIVATE_SLOT( d, void imageRequestFinished( KJob* ) )
};

#endif
