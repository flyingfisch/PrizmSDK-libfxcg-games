// ui.h
/// @file
/// @brief Miscellaneous user-interface

#ifndef UI_H
#define UI_H

enum
{
	LIGHTING_UNEXPLORED = 0,
	LIGHTING_DARK,
	LIGHTING_LIT,
	LIGHTING_UNINITIALIZED
};

typedef struct
{
	uchar tile;
	uchar lighting;
#ifdef SUPPORT_COLOR
	colorinfo color;
#endif
} UI_TF_TileInfo;

typedef struct
{
	// Base properties: How big the TF would be on a large map
	coord full_pos, full_dimensions, full_window;
	const uchar *graphics;
	uchar tileheight, tilewidth;
	
	// How big the map is
	coord size;
	
	// How big the TF is on *this* map
	coord pos, window, dimensions;
	
	// Data (changes with use)
	coord viewoffset;
	
} UI_Tilefield;

#endif //UI_H

