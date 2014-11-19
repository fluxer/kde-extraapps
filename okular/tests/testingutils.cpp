/***************************************************************************
 *   Copyright (C) 2013 by Jon Mease <jon.mease@gmail.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "testingutils.h"
#include "core/annotations.h"
#include <qtest_kde.h>

namespace TestingUtils
{
    QString getAnnotationXml(const Okular::Annotation* annotation)
    {
        QString annotXmlString;
        QTextStream stream(&annotXmlString, QIODevice::Append);
        annotation->getAnnotationPropertiesDomNode().save(stream, 0);
        return annotXmlString;
    }

    bool pointListsAlmostEqual( QLinkedList< Okular::NormalizedPoint > points1, QLinkedList< Okular::NormalizedPoint > points2 ) {

        QLinkedListIterator<Okular::NormalizedPoint> it1( points1 );
        QLinkedListIterator<Okular::NormalizedPoint> it2( points2 );
        while ( it1.hasNext() && it2.hasNext() )
        {
            const Okular::NormalizedPoint& p1 = it1.next();
            const Okular::NormalizedPoint& p2 = it2.next();
            if ( !qFuzzyCompare( p1.x, p2.x ) || !qFuzzyCompare( p1.y, p2.y ) ) {
                return false;
            }
        }
        return !it1.hasNext() && !it2.hasNext();
    }

    QString AnnotationDisposeWatcher::m_disposedAnnotationName = QString();

    QString AnnotationDisposeWatcher::disposedAnnotationName() {
        return m_disposedAnnotationName;
    }

    void AnnotationDisposeWatcher::resetDisposedAnnotationName()
    {
        m_disposedAnnotationName = QString();
    }

    void AnnotationDisposeWatcher::disposeAnnotation( const Okular::Annotation *ann )
    {
        m_disposedAnnotationName = ann->uniqueName();
    }
}
