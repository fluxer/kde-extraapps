/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "sha1hashgen.h"
#include <stdio.h>
#include <string.h>
#include "functions.h"



namespace bt
{
	SHA1HashGen::SHA1HashGen() : h(new QCryptographicHash(QCryptographicHash::Sha1))
	{
		memset(result,9,20);
	}


	SHA1HashGen::~SHA1HashGen()
	{
		delete h;
	}

	SHA1Hash SHA1HashGen::generate(const Uint8* data,Uint32 len)
	{
		QCryptographicHash hash(QCryptographicHash::Sha1);
		hash.addData((const char*)data,len);
		return SHA1Hash((const bt::Uint8*)hash.result().constData());
	}
	
	void SHA1HashGen::start()
	{
		h->reset();
	}
		
	void SHA1HashGen::update(const Uint8* data,Uint32 len)
	{
		h->addData((const char*)data,len);
	}
		
	 
	void SHA1HashGen::end()
	{
		memcpy(result,h->result().constData(),20);
	}
		

	SHA1Hash SHA1HashGen::get() const
	{
		return SHA1Hash(result);
	}
}
