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
// rle.c
/// @file
/// @brief Code for run-length-encoding compression.
///
/// THIS FILE IS USED IN PROGRAMS OTHER THAN CALCROGUE, AND SHOULD NOT INCLUDE
/// CROGUE.H

#ifdef TARGET // Compiling the main program
#include "crogue.h"
#endif

#include "substdio.h"
#include "config.h"
#include "rle.h"
#include "huffman.h"

#ifdef DISABLE_COMPRESSION
#define DISABLE_RLE
#endif

#define CHECKSUM_INIT 0xF1897134L
#define CHECKSUM_MUL  0xA8159803L
#define CHECKSUM_ADD  0x81340967L
static unsigned short _runlen=0;
static unsigned char _runchar=' ';

static unsigned long _checksum_file(FILE *f);

static unsigned char _rleread_file(void);
static unsigned char _rleread_ptr(void);
static unsigned char _rleread_huffman(void);

static const void *_rlepos;
static unsigned char (*_rleread)(void);
#ifdef CPROTO //HACKHACK - for some reason, cproto's choking
static void *_putc_compress;
#else
static void (*_putc_compress)(unsigned char c, FILE *f) = (void(*)(unsigned char c, FILE *f))&fputc;
#endif

static const unsigned char magic_byte = 0x5F;

//{{{
unsigned long checksum_bytes(void *data, unsigned int length)
{
	unsigned long checksum = CHECKSUM_INIT;
	unsigned i;
	
	for(i=0; i<length; i++)
	{
		checksum += ((unsigned char*)data)[i];
		checksum *= CHECKSUM_MUL;
		checksum += CHECKSUM_ADD;
	}
	
	return checksum;
}
//}}}
//{{{
static unsigned long _checksum_file(FILE *f)
{
#ifdef IS_CALCULATOR
	return checksum_bytes((char*)f->base+6, peek_w(f->base)-6);
#else
	return 0;
#endif
}
//}}}

//{{{
// Verify the checksum on a file
int frleverify(FILE *f)
{
#ifdef IS_CALCULATOR
	return _checksum_file(f) == peek_l( (char*)f->base + 2 );
#else
	return 1; // Stubbed on PC
#endif
}
//}}}

//{{{
void frlewrite(void *data, size_t s, size_t granularity, FILE *f)
{
	unsigned long i, j;
	
#ifdef DEBUG_FILES
	unsigned long checksum;
#endif
	
#ifdef DEBUG_FILES
	checksum = checksum_bytes(data, s);
#	ifdef DISABLE_COMPRESSION
		fwrite(&checksum, sizeof(long), 1, f);
#	else
		frleflush(f);
		for(i=0; i<sizeof(checksum); i++)
			_putc_compress( (char)(checksum>>(8*i)), f );
#	endif
#endif
	
#ifdef DISABLE_COMPRESSION
	fwrite(data, s, 1, f);
#else
#	ifdef DISABLE_RLE
		for(i=0; i<s; i++)
			_putc_compress( ((unsigned char*)data)[i], f );
#	else
		for(i=0; i<granularity; i++)
		for(j=i; j<s; j+=granularity)
			frleputc( ((unsigned char*)data)[j], f);
#	endif
#endif
}
//}}}
//{{{
void frleread(void *data, size_t s, size_t granularity, FILE *f)
{
	unsigned long i, j;
#ifdef DEBUG_FILES
	unsigned long checksum_read, checksum_validate;
#endif
	
#ifndef DISABLE_COMPRESSION
	rle_set_source(rle_huffman, f);
#endif
	
#ifdef DEBUG_FILES
#	ifdef DISABLE_COMPRESSION
		fread(&checksum_read, sizeof(long), 1, f);
#	else
		checksum_read = 0;
		for(i=0; i<sizeof(checksum_read); i++)
			checksum_read += rlegetc() << (8*i);
#	endif
#endif

#ifdef DISABLE_COMPRESSION
	fread(data, s, 1, f);
#else
#	ifdef DISABLE_RLE
		for(i=0; i<s; i++)
			((unsigned char*)data)[i] = _rleread();
#	else
		for(i=0; i<granularity; i++)
		for(j=i; j<s; j+=granularity)
			((char*)data)[j] = rlegetc();
#	endif
#endif
	
#ifdef DEBUG_FILES
	checksum_validate = checksum_bytes(data, s);
	if(checksum_read != checksum_validate) {
#		ifndef EXTERNAL_TOOL
			message( gettext(
			"File checksum doesn't match! %lp vs %lp, size=%i, granularity=%i"),
				   checksum_read, checksum_validate, s, granularity);
#		endif
	}
#endif
}
//}}}

//{{{
void frleinit(void)
{
	_runlen=0;
	_runchar=0;
}
//}}}
//{{{
void frleinit_read(FILE *f)
{
	frleinit();
	fseek(f, 4, SEEK_SET);
}
//}}}
//{{{
void frleinit_write(FILE *f, void (*outfunc)(unsigned char c, FILE *f))
{
	unsigned long padding = 0; // Added to the beginning to make room for a checksum
	frleinit();
	_putc_compress = outfunc;
	
	// Make space at the beginning for a checksum
	fwrite(&padding, sizeof padding, 1, f);
}
//}}}
//{{{
void frleinit_write_nochecksum(void (*outfunc)(unsigned char c, FILE *f))
{
	frleinit();
	_putc_compress = outfunc;
}
//}}}
//{{{
void frlefinalize(FILE *f)
{
	unsigned long checksum;
	
	frleflush(f);
	
	checksum = _checksum_file(f);
	rewind(f);
	fwrite(&checksum, sizeof checksum, 1, f);
}
//}}}
//{{{
void frleflush(FILE *fout)
{
	if(_runlen > 127)
	{
		_putc_compress( magic_byte, fout );
		_putc_compress( _runchar, fout );
		_putc_compress( (unsigned char)((_runlen>>8)|0x80), fout );
		_putc_compress( (unsigned char)(_runlen & 0xFF), fout );
	}
	else if(_runlen>2 || _runchar==magic_byte) // encode only runs that are long enough to be profitable
		// encode only runs that are long enough to be profitable
		// (but the magic byte MUST be encoded)
	{
		_putc_compress( magic_byte, fout );
		_putc_compress( _runchar, fout );
		_putc_compress( (unsigned char)_runlen, fout );
	}
	else if(_runlen==2)
	{
		_putc_compress( _runchar, fout );
		_putc_compress( _runchar, fout );
	}
	else if(_runlen==1)
	{
		_putc_compress( _runchar, fout );
	}
	
	_runlen = 0;
	_runchar = 0;
}
//}}}
//{{{
//
// Writes a run-length encoded character to a file
// Uses static data to keep track of state; the 'mode' parameter
// is used to indicate special conditions
// mode: 0 for normal write, 1 for end of write, 2 for initialize
//
void frleputc(unsigned char c, FILE *fout)
{
	if( _runlen == 0 )
	{
		_runlen=1;
		_runchar=c;
		return;
	}
	
	if( _runlen >= 0x7FFF || c != _runchar )
	{
		frleflush(fout);
		_runchar = c;
		_runlen = 1;
	} else
		_runlen++;
}
//}}}
//{{{
//
// Reads a run-length encoded character from a file
// Uses static data to keep track of state
//
unsigned char rlegetc(void)
{
	unsigned char in;
	
	if( !_runlen ) {
		in = _rleread();
		
		if(in!=magic_byte)
			return in;
		
		_runchar = _rleread();
		_runlen = _rleread();
		if(_runlen & 0x80) {
			_runlen &= ~0x80;
			_runlen <<= 8;
			_runlen |= _rleread();
		}
	}
	_runlen--;
	return _runchar;
}
//}}}

//{{{
void rle_set_source(rle_source_type t, const void *s)
{
	_rlepos = s;
	
	switch(t)
	{
		case rle_huffman:
			_rleread = _rleread_huffman;
			break;
		case rle_file:
			_rleread = _rleread_file;
			break;
		case rle_pointer:
			_rleread = _rleread_ptr;
			break;
	}
}
//}}}
//{{{
static unsigned char _rleread_huffman(void)
{
	return fhuffmangetc( (FILE*)_rlepos );
}
//}}}
//{{{
static unsigned char _rleread_file(void)
{
	return fgetc( (FILE*)_rlepos );
}
//}}}
//{{{
static unsigned char _rleread_ptr(void)
{
	const unsigned char *rlepos = _rlepos;
	unsigned char ret = *(rlepos++);
	_rlepos = rlepos;
	return ret;
}
//}}}

