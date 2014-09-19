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
// tilefield.c
/// @file
/// @brief Main map display

#include "crogue.h"

static UI_Tilefield tf;

//{{{
void UI_TF_MoveCamera(int x, int y)
{
	int oldx=tf.viewoffset.x, oldy=tf.viewoffset.y;
		// Previous x,y offset, to compare later to see if it's changed
	uint threshhold_x, threshhold_y;
	
	threshhold_x = tf.window.x / 3 - 1;
	threshhold_y = tf.window.y / 3 - 1;
	
	// Scroll X
	if( x-threshhold_x <= tf.viewoffset.x || x+threshhold_x-tf.viewoffset.x >= tf.window.x )
	{
		tf.viewoffset.x = max(x - ((sint)tf.window.x)/2, 0);
		if(tf.viewoffset.x + tf.window.x > tf.size.x)
			tf.viewoffset.x = tf.size.x - tf.window.x;
	}
	
	// Scroll Y
	if( y-threshhold_y <= tf.viewoffset.y || y+threshhold_y-tf.viewoffset.y >= tf.window.y )
	{
		tf.viewoffset.y = max(y - ((sint)tf.window.y)/2, 0);
		if(tf.viewoffset.y + tf.window.y > tf.size.y)
			tf.viewoffset.y = tf.size.y - tf.window.y;
	}
	
	if( tf.viewoffset.x != oldx || tf.viewoffset.y != oldy ) UI_TF_FullRedraw();
}
//}}}
//{{{
void UI_TF_PlaceCamera(int x, int y)
{
	tf.viewoffset.x = max(x - ((sint)tf.window.x)/2, 0);
	tf.viewoffset.y = max(y - ((sint)tf.window.y)/2, 0);
	if(tf.viewoffset.x + tf.window.x > tf.size.x)
		tf.viewoffset.x = tf.size.x - tf.window.x;
	if(tf.viewoffset.y + tf.window.y > tf.size.y)
		tf.viewoffset.y = tf.size.y - tf.window.y;
	return;
}
//}}}
//{{{
void UI_TF_Scroll(int x, int y)
{
	signed int new_x, new_y;
	
	if(w->time == 0)
	{
		message(gettext("Use the number keys (1-9) to move."));
		return;
	}
	
	new_x = tf.viewoffset.x + x;
	new_y = tf.viewoffset.y + y;
	if(new_x<0) new_x = 0;
	if(new_y<0) new_y = 0;
	if(new_x + tf.window.x > tf.size.x) new_x = tf.size.x - tf.window.x;
	if(new_y + tf.window.y > tf.size.y) new_y = tf.size.y - tf.window.y;
	
	tf.viewoffset.x = new_x;
	tf.viewoffset.y = new_y;
	
	UI_TF_FullRedraw();
}
//}}}
//{{{
void UI_TF_Init(coord size)
{
	tf.size = size;
	tf.viewoffset = COORD(0,0);
	
	// Recalculate pos and window
	tf.window = tf.full_window;
	tf.pos = tf.full_pos;
	if(tf.window.x > tf.size.x)
		tf.window.x = tf.size.x;
	if(tf.window.y > tf.size.y)
		tf.window.y = tf.size.y;
	
	tf.dimensions.x = tf.window.x * tf.tilewidth;
	tf.dimensions.y = tf.window.y * tf.tileheight;
	tf.pos.x += (tf.full_dimensions.x-tf.dimensions.x)/2;
	tf.pos.y += (tf.full_dimensions.y-tf.dimensions.y)/2;
}
//}}}
//{{{
void UI_TF_Place(coord pos, coord dimensions, const uchar *graphics,
	uchar tileheight, uchar tilewidth)
{
	tf.full_pos = pos;
	tf.full_dimensions = dimensions;
	tf.graphics = graphics;
	tf.tileheight = tileheight;
	tf.tilewidth = tilewidth;
	tf.full_window.x = tf.full_dimensions.x / tf.tilewidth;
	tf.full_window.y = tf.full_dimensions.y / tf.tileheight;
	
	UI_TF_Init(tf.size);
}
//}}}
//{{{
void UI_TF_Deinit(void)
{
}
//}}}
//{{{
void UI_TF_PutTile(int x, int y, uchar tnum, uchar lighting)
{
	int tx, ty;
	UI_TF_TileInfo tile;
	tx = x - tf.viewoffset.x;
	ty = y - tf.viewoffset.y;
	
	tile.tile = tnum;
	tile.lighting = lighting;
	
	if(tx<0 || ty<0 || tx>=tf.window.x || ty>=tf.window.y) return;
	
	Graph_DrawTile(&tf, tx, ty, tile);
}
//}}}
#ifdef SUPPORT_COLOR
//{{{
void UI_TF_PutTile_Color(int x, int y, uchar tnum, uchar lighting, colorinfo color)
{
	int tx, ty;
	UI_TF_TileInfo tile;
	tx = x - tf.viewoffset.x;
	ty = y - tf.viewoffset.y;
	
	tile.tile = tnum;
	tile.lighting = lighting;
	tile.color = color;
	
	if(tx<0 || ty<0 || tx>=tf.window.x || ty>=tf.window.y) return;
	
	Graph_DrawTile(&tf, tx, ty, tile);
}
//}}}
#endif
//{{{
void UI_TF_FullRedraw()
{
	int i, ii;
	
	// DEBUG: Do this in reverse-order to reveal overclear
	for(i=0; i<tf.window.y; i++)
	{
		for(ii=0; ii<tf.window.x; ii++)
			draw_tile(ii+tf.viewoffset.x, i+tf.viewoffset.y);
	}
	
	// Top dead zone
	if(tf.pos.y > tf.full_pos.y) {
		Graph_ClearRect(tf.full_pos.x, tf.full_pos.y,
		                tf.full_pos.x + tf.full_dimensions.x, 
		                tf.pos.y-1);
	}
	// Bottom dead zone
	if(tf.dimensions.y < tf.full_dimensions.y) {
		Graph_ClearRect(tf.full_pos.x, tf.pos.y+tf.dimensions.y, 
			tf.full_pos.x + tf.full_dimensions.x,
			tf.full_pos.y + tf.full_dimensions.y);
	}
	// Left and right dead zones
	if(tf.window.x < tf.full_window.x) {
		Graph_ClearRect(tf.full_pos.x,
		                tf.pos.y,
		                tf.pos.x-1,
		                tf.pos.y + tf.dimensions.y);
		Graph_ClearRect(tf.pos.x + tf.window.x*tf.tilewidth,
		                tf.pos.y,
		                tf.full_pos.x + tf.full_dimensions.x,
		                tf.pos.y + tf.dimensions.y);
	}
	
//	for(i=0; i<tf.window.y; i++)
//	{
//		for(ii=0; ii<tf.window.x; ii++)
//			draw_tile(ii+tf.viewoffset.x, i+tf.viewoffset.y);
//	}
}
//}}}

#ifdef REALCOMPUTER
//{{{
void UI_TF_PlaceCursor(int x, int y)
{
	move(y+tf.pos.y-tf.viewoffset.y, x+tf.pos.x-tf.viewoffset.x);
}
//}}}
#endif
//{{{
void UI_TF_DrawEffect(sshort x, sshort y, ushort tnum)
{
	UI_TF_TileInfo t;
	t.tile = tnum;
	if(!(w->t[y][x].flags & TFLAG_EXPLORED))
		t.lighting = LIGHTING_UNEXPLORED;
	else if(w->t[y][x].flags & TFLAG_LIT)
		t.lighting = LIGHTING_LIT;
	else
		t.lighting = LIGHTING_DARK;
	
#ifdef SUPPORT_COLOR
	t.color.lit = t.color.dark = COLOR_BOLD + 7;
#endif
#ifdef USE_CURSES
	UI_TF_PlaceCursor(x, y);
#endif
	x -= tf.viewoffset.x;
	y -= tf.viewoffset.y;
	Graph_DrawTile(&tf, x, y, t);
}
//}}}
//{{{
void UI_TF_ClearEffects(void)
{
	UI_TF_FullRedraw();
}
//}}}

