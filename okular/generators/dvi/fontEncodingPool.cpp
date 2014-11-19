// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
// fontEncodingPool.cpp
//
// Part of KDVI - A DVI previewer for the KDE desktop environment
//
// (C) 2003 Stefan Kebekus
// Distributed under the GPL

#include <config.h>

#ifdef HAVE_FREETYPE

#include "fontEncodingPool.h"


fontEncodingPool::fontEncodingPool()
{}

fontEncodingPool::~fontEncodingPool()
{
  qDeleteAll(dictionary);
}

fontEncoding *fontEncodingPool::findByName(const QString &name)
{
  fontEncoding *ptr = dictionary.value( name );

  if (ptr == 0) {
    ptr = new fontEncoding(name);
    if (ptr->isValid())
      dictionary.insert(name, ptr );
    else {
      delete ptr;
      ptr = 0;
    }
  }

  return ptr;
}

#endif // HAVE_FREETYPE
