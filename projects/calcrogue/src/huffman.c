/* {{{
 * CalcRogue, a roguelike game for PCs, calculators and PDAs
 * Copyright (C) 2003 Jim Babcock
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * }}} */
// huffman.c
/// @file
/// @brief Implements the Huffman compression algorithm
///
/// Uses a pre-generated symbol table (hufftable.h).

#ifdef TARGET // Compiling CalcRogue (not a tool)
#include "crogue.h"
#endif

#include "hufftable.h"
#include "huffman.h"
#include "substdio.h"

#ifdef DISABLE_COMPRESSION
#define DISABLE_HUFFMAN
#endif

static unsigned long _hp_pending=0;
static unsigned short _hp_pending_length=0;

void fhuffmaninit_write(void)
{
	_hp_pending = 0;
	_hp_pending_length = 0;
}

void fhuffmanputc(unsigned char c, FILE *f)
{
#ifdef DISABLE_HUFFMAN
	fputc(c, f);
#else
	
	_hp_pending_length += huff_compress.replen[c];
	_hp_pending |= (long)huff_compress.rep[c] << (32L - _hp_pending_length);
	if(_hp_pending_length >= 16)
		fhuffmanflush(f);
#endif
}

void fhuffmanflush(FILE *f)
{
#ifndef DISABLE_HUFFMAN
	fputc((char)(_hp_pending>>24L), f);
	fputc((char)(_hp_pending>>16L), f);
	_hp_pending <<= 16;
	if(_hp_pending_length >= 16)
		_hp_pending_length -= 16;
	else
		_hp_pending_length = 0;
#endif
}

static unsigned long _hg_pending=0;
static unsigned short _hg_pending_length=0;

void fhuffmaninit_read(void)
{
	_hg_pending = 0;
	_hg_pending_length = 0;
}

char fhuffmangetc(FILE *f)
{
#ifdef DISABLE_HUFFMAN
	return fgetc(f);
#else
	short high, low, mid;
	unsigned short searchtarget;
	
	while(_hg_pending_length < 24)
	{
		_hg_pending <<= 8;
		_hg_pending_length += 8;
		if(!feof(f))
			_hg_pending |= fgetc(f);
	}
	searchtarget = _hg_pending >> (_hg_pending_length - 16UL);
	low = 0;
	high = 255;
	
	do
	{
		mid = (low+high+1)/2;
		if(huff_decompress.entries[mid].binrep > searchtarget)
			high = mid-1;
		else
			low = mid;
	} while(high>low);
	
	
	_hg_pending <<= (32L + huff_decompress.entries[low].binreplen - _hg_pending_length);
	_hg_pending_length -= huff_decompress.entries[low].binreplen;
	_hg_pending >>= (32L - _hg_pending_length);
	
	return huff_decompress.entries[low].byterep;
#endif
}


