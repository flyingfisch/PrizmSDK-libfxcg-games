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
// draw.c
/// @file
/// @brief Base code for drawing user interface

#include "crogue.h"

//{{{
static const char* hunger_status(void)
{
	if(w->plr.satiation <= -600)
		return gettext(" starving");
	else if(w->plr.satiation <= 600)
		return gettext(" weak");
	else if(w->plr.satiation < 2400)
		return gettext(" hungry");
	else
		return "";
}
//}}}

static int statuslinepos;
static int statusheight;
//{{{
void UI_Status_Place(int y, int h)
{
	statuslinepos = y;
	statusheight = h;
}
//}}}

//{{{
void full_redraw(void)
{
	clear_dead_space();
#ifdef PALMOS
	buttonbar_draw();
#endif
	UI_TF_FullRedraw();
	w->messagevis++;
	draw();
}
//}}}
//{{{
void draw(void)
{
	char outbuf[64];
	
	if(w->messagevis)
	{
		if(!(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_PARALYZED))
			w->messagevis--;
	}
	else
		UI_MF_clear();
	
	// Draw statusbar
	if(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_NUMB)
		sprintf(outbuf, gettext("HP\?\?/%i "), w->plr.hps_max);
	else
		sprintf(outbuf, gettext("HP%i/%i "), w->plr.hps, w->plr.hps_max);
	
	catprintf(outbuf, gettext("PW%i/%i LV%i $:%li %s%s%s%s"),
		w->plr.pps,
		w->plr.pps_max,
		w->plr.level,
		player_gold(),
		(const char*)deref_file_ptr(MAPDESC_CURRENT.name),
		hunger_status(),
		(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_CONFUSED) ?
			gettext(" conf") : "",
		(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_HALLUCINATING) ?
			gettext(" hallu") : ""
	);

#ifdef IS_CALCULATOR
	Graph_ClearRect(0, statuslinepos,
	                SCREEN_WIDTH-1, statuslinepos+statusheight-1);
	SetFont(get_option(OPTION_FONTSIZE));
	Graph_DrawStr(0, statuslinepos, outbuf);
#endif
#ifdef PALMOS
	Graph_ClearRect(0, statuslinepos,
	                SCREEN_WIDTH-1, statuslinepos+statusheight-1);
	Graph_DrawStr(0, statuslinepos, outbuf);
#endif
#ifdef REALCOMPUTER
	clear_line(statuslinepos);
	clear_line(statuslinepos+1);
	mvaddstr(statuslinepos, 0, outbuf);
	move(w->plr.y+2, w->plr.x);
#endif
#ifdef REALCOMPUTER // Place the cursor over the player
	UI_TF_PlaceCursor(w->plr.x, w->plr.y);
#endif
}
//}}}
//{{{
void draw_all_tiles(void)
{
	int x, y;
	
	for(y=0; y<MAPSIZE_Y; y++)
	for(x=0; x<MAPSIZE_X; x++)
		draw_tile(x, y);	
}
//}}}

//{{{
void draw_tile(ushort x, ushort y)
{
	uchar tilenum=1;
	int flags;
	uchar lighting=LIGHTING_DARK;
#	ifdef SUPPORT_COLOR
	colorinfo color = {WHITE|COLOR_BOLD, WHITE|COLOR_BOLD};
#	endif
	sint monstnum;
	
	flags = w->t[y][x].flags;
	
	if( !(flags & TFLAG_EXPLORED) ) {
		tilenum = 1;
		lighting = LIGHTING_UNEXPLORED;
		goto done;
	}
	
	if(flags & TFLAG_LIT)
		lighting = LIGHTING_LIT;
	
	if((flags & (TFLAG_OCCUPIED|TFLAG_LIT)) == (TFLAG_OCCUPIED|TFLAG_LIT))
	{
		if(x==w->plr.x && y==w->plr.y) {
			tilenum = '@';
			goto done;
		} else {
			monstnum = monstbytile(x, y);
			if( monstnum >= 0 && player_can_see(monstnum) )
			{
				if(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_HALLUCINATING) {
					tilenum = RANGE('Z', 'A');
#					ifdef SUPPORT_COLOR
					color.lit = RANGE(7,1) | COLOR_BOLD;
#					endif
				} else {
					tilenum = MDESC(monstnum)->drawchar;
#					ifdef SUPPORT_COLOR
					color = MDESC(monstnum)->color;
#					endif
				}
				goto done;
			}
		}
	}
	
	if(flags & TFLAG_ITEM) {
		tilenum = draw_item(x, y);
#		ifdef SUPPORT_COLOR
		color = color_item(x, y);
#		endif
	} else {
		if(flags & TFLAG_HIDETRAP)
		{
			tilenum = w->tiledescs[TILE_FLOOR].drawchar;
#			ifdef SUPPORT_COLOR
			color = w->tiledescs[TILE_FLOOR].color;
#			endif
		}
		else
		{
			tilenum = w->tiledescs[w->t[y][x].type].drawchar;
#			ifdef SUPPORT_COLOR
			color = w->tiledescs[w->t[y][x].type].color;
#			endif
		}
	}
	
done:
#	ifdef SUPPORT_COLOR
	UI_TF_PutTile_Color(x, y, tilenum, lighting, color);
#	else
	UI_TF_PutTile(x, y, tilenum, lighting);
#	endif
}
//}}}

// Some areas in the UI aren't covered by any widget. The layout manager
// (#init_layout) should report these as 'dead spaces' so they can be cleared
// when something covers them.
static struct { unsigned char left, top, right, bottom; } dead_spaces[4];
static int num_dead_spaces = 0;
//{{{
void add_dead_zone(int left, int top, int right, int bottom)
{
	dead_spaces[num_dead_spaces].left   = left;
	dead_spaces[num_dead_spaces].top    = top;
	dead_spaces[num_dead_spaces].right  = right;
	dead_spaces[num_dead_spaces].bottom = bottom;
	num_dead_spaces++;
}
//}}}
//{{{
void clear_dead_space(void)
{
	int ii;
	for(ii=0; ii<num_dead_spaces; ii++)
	{
		Graph_ClearRect(dead_spaces[ii].left, dead_spaces[ii].top,
		                dead_spaces[ii].right, dead_spaces[ii].bottom);
	}
}
//}}}

