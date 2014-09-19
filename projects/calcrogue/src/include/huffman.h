// huffman.h
/// @file
/// @brief Huffman compression
/// @sa src/huffman.c

#ifndef HUFFMAN_H
#define HUFFMAN_H
#include "substdio.h"

typedef struct hufftable
{
	unsigned short rep[256];
	unsigned char replen[256];
} hufftable;

typedef struct huffdecompresstable_entry
{
	unsigned short binrep; // Occupies the TOP of the word (pad bits at the bottom)
	unsigned char binreplen;
	unsigned char byterep;
} huffdecompresstable_entry;

typedef struct huffdecompresstable
{
	huffdecompresstable_entry entries[256]; // sorted
} huffdecompresstable;


void fhuffmaninit_write(void);
void fhuffmanflush(FILE *f);
void fhuffmaninit_read(void);
void fhuffmanputc(unsigned char c, FILE *f);
char fhuffmangetc(FILE *f);

#endif //HUFFMAN_H

