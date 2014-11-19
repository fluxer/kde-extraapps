// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
//
// Extracted from:
// Class: documentRenderer
//
// Abstract Widget for displaying document types
// Needs to be implemented from the actual parts
// using kviewshell
// Part of KViewshell - A generic interface for document viewers.
//
// (C) 2004-2005 Wilfried Huss
// (C) 2004-2006 Stefan Kebekus.
//  Distributed under the GPL.

#include "dviRenderer.h"

SimplePageSize dviRenderer::sizeOfPage(const PageNumber& page)
{
#if !defined(QT_NO_THREAD)
  // Wait for all access to this DocumentRenderer to finish
  //QMutexLocker locker(&mutex);
#endif

  if (!page.isValid())
    return SimplePageSize();
  if (page > totalPages())
    return SimplePageSize();
  if (page > pageSizes.size())
    return SimplePageSize();

  return pageSizes[page-1];
}

Anchor dviRenderer::findAnchor(const QString &locallink)
{
  QMap<QString,Anchor>::Iterator it = anchorList.find(locallink);
  if (it != anchorList.end())
    return *it;
  else
    return Anchor();
}


PageNumber dviRenderer::totalPages() const
{
  PageNumber temp = numPages;
  return temp;
}

