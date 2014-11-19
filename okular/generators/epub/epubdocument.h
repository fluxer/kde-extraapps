/***************************************************************************
 *   Copyright (C) 2008 by Ely Levy <elylevy@cs.huji.ac.il>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef EPUB_DOCUMENT_H
#define EPUB_DOCUMENT_H

#include <QTextDocument>
#include <QUrl>
#include <QVariant>
#include <QImage>
#include <kurl.h>
#include <epub.h>

namespace Epub {

  class EpubDocument : public QTextDocument {

  public:
    EpubDocument(const QString &fileName);
    bool isValid();
    ~EpubDocument();
    struct epub *getEpub();
    void setCurrentSubDocument(const QString &doc);
    int maxContentHeight() const;
    int maxContentWidth() const;
    enum Multimedia { MovieResource = 4, AudioResource = 5 };

  protected:
    virtual QVariant loadResource(int type, const QUrl &name);

  private:
    void checkCSS(QString &css);

    struct epub *mEpub;
    KUrl mCurrentSubDocument;

    int padding;

    friend class Converter;
  };

}
#endif
