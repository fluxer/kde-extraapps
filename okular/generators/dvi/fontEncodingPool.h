// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
// fontEncodingPool.h
//
// Part of KDVI - A DVI previewer for the KDE desktop environment
//
// (C) 2003 Stefan Kebekus
// Distributed under the GPL

#ifndef _FONTENCODINGPOOL_H
#define _FONTENCODINGPOOL_H

#include "fontEncoding.h"

#include <QHash>

class QString;


class fontEncodingPool {
 public:
  fontEncodingPool();
  ~fontEncodingPool();

  fontEncoding *findByName(const QString &name);

 private:
  QHash<QString,fontEncoding*> dictionary;
};

#endif
