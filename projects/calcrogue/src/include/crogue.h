// crogue.h
/// @file
/// @brief Base include file, which includes everything else

#ifndef CROGUE_H
#define CROGUE_H

#include "config.h"
#include "machdep.h"

//{{{
// Description of a monster
#define MDESC(a)            find_monster_description(a)
#define MATTACK(a, b)       (MDESC(a)->attacks[b])

#define MAPDESC_CURRENT     (*w->current_map)
#define TILEDESC(a)         (w->tiledescs[(a).type])

#define MAPSPEC(a)          (MAPDESC_CURRENT.ext[a])
#define TILESPEC(x,y)       MAPSPEC(w->t[(y)][(x)].special)
//}}}

// Bilingual header support
// Certain header files need to be understood when read either from the
// main program (C), or the data file (custom language). Use these macros
// to hide features which the data file won't understand.
#define CONST const
#ifdef PALMOS
	#define ECONST
#else
	#define ECONST const
#endif

#include "util.h"
#include "rle.h"
#include "huffman.h"
#include "map.h"
#include "items.h"
#include "monst.h"
#include "ui.h"
#include "keyboard.h"
#include "timer.h"
#include "player.h"
#include "score.h"
#include "options.h"
#include "constdata.h"
#include "world.h"
#include "interpret.h"

#ifndef CPROTO
#include "automatic.h"
#include "dll_import.h"
#include "dll_export.h"
#endif

#ifndef DATA_FILE
#include "data_export.h"
#endif

// Game data
#ifdef REGISTERVAR
	register world *w asm("a4");
	extern world *w_non_reg;
#else
	extern world *w;
#endif

// Constant data
#ifdef IS_CALCULATOR
extern ECONST uchar tiles_5x5[128*5];
extern ECONST uchar tiles_7x7[128*7];
#define blacktile 1
#define whitetile 2
#else
extern ECONST uchar tiles[128];		// For PC, this is mapping from calc-side
                                    // tiles to extended ASCII
#endif

extern ECONST schar dir[8][2];	// 8 cardinal directions converted into (x,y)
                                // form
#ifndef SEG
#	define SEG(a)
#endif

#ifndef CPROTO
extern ECONST import_functions body_functions;
#include "proto.h"
#endif

#endif //CROGUE_H

