// rle.h
/// @file
/// @brief RLE compression
/// @sa rle.c

#ifndef RLE_H
#define RLE_H

#include "substdio.h"

void frleputc(unsigned char c, FILE *fout);
unsigned char rlegetc(void);

unsigned long checksum_bytes(void *data, unsigned int length);
int frleverify(FILE *f);
void frleinit_read(FILE *f);
void frleinit_write(FILE *f, void (*outfunc)(unsigned char c, FILE *f));
void frleinit_write_nochecksum(void (*outfunc)(unsigned char c, FILE *f));

void frleinit(void);
void frleflush(FILE *fout);
void frlewrite(void *data, size_t s, size_t n, FILE *f);
void frleread(void *data, size_t s, size_t n, FILE *f);
void frlefinalize(FILE *f);


typedef enum
{
	rle_file,
	rle_huffman,
	rle_pointer
} rle_source_type;

void rle_set_source(rle_source_type t, const void *s);

	// Prototyped here for shared-function purposes
void panic(const char *fmt, ...);

#endif
