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
// light.c
/// @file
/// @brief Functions for line-of-sight and light sources
///
/// The functions in this file are speed-critical, and get optimized
/// accordingly. Don't put anything big in here, and keep the functions
/// fast.

#include "crogue.h"

#define MAX_LIGHT_RADIUS 15
#define MAX_LIGHT_RADIUS_SQUARED 225

static void calc_light_tile(int x, int y);

//{{{
void calc_light(void)
/*
 * As you can see, this function is *ugly*. That's because the problem domain
 * is a little ugly, and because it's speed critical.
 */
{
	int xi, yi;
//	int min_x, min_y;
//	int max_x, max_y;
//	
//	min_x = w->plr.x - MAX_LIGHT_RADIUS;
//	if(min_x < 0) min_x = 0;
//	max_x = w->plr.x + MAX_LIGHT_RADIUS;
//	if(max_x >= MAPSIZE_X) max_x = MAPSIZE_X - 1;
//	min_y = w->plr.y - MAX_LIGHT_RADIUS;
//	if(min_y < 0) min_y = 0;
//	max_y = w->plr.y + MAX_LIGHT_RADIUS;
//	if(max_y >= MAPSIZE_Y) max_y = MAPSIZE_Y - 1;
//	
//	for(yi=min_y; yi<=max_y; yi++)
//	for(xi=min_x; xi<=max_x; xi++)
//	{
//		calc_light_tile(xi, yi);
//	}
	for(yi=0; yi<MAPSIZE_Y; yi++)
	for(xi=0; xi<MAPSIZE_X; xi++)
		calc_light_tile(xi, yi);
}
//}}}
//{{{
static void calc_light_tile(int x, int y)
{
	int distance;
	int is_in_range;
	int change;
	
	distance = distancesquare(x, y, w->plr.x, w->plr.y);
	
	if(distance <= w->plr.extrinsic[STAT_LIGHTRADIUS])
		is_in_range = 1;
	else
		is_in_range = 0;
	
	// Find lit tiles to unlight
	if(w->t[y][x].flags & TFLAG_LIT)
	{
		if(w->t[y][x].flags & TFLAG_INTRINSIC_LIGHT)
		{
			if( distance > MAX_LIGHT_RADIUS_SQUARED ||
				!tracevision(x, y, w->plr.x, w->plr.y) )
			{
				w->t[y][x].flags &= ~TFLAG_LIT;
				draw_tile(x, y);
			}
		} else {
			if( !is_in_range ||
				!tracevision(x, y, w->plr.x, w->plr.y) )
			{
				w->t[y][x].flags &= ~TFLAG_LIT;
				draw_tile(x, y);
			}
		}
	}
	// Find unlit tiles to light
	else if(!(w->t[y][x].flags & TFLAG_LIT))
	{
		change = 0;
		
		if(w->t[y][x].flags & TFLAG_INTRINSIC_LIGHT)
		{
			if(distance > MAX_LIGHT_RADIUS_SQUARED) return;
			
			if(TILEDESC(w->t[y][x]).transparent) {
				change = tracevision(x, y, w->plr.x, w->plr.y);
			} else if(is_in_range) {
				change = tracevision(x, y, w->plr.x, w->plr.y);
			} else {
				int xd=x, yd=y;
				
				if(w->plr.x > x)      xd++;
				else if(w->plr.x < x) xd--;
				if(w->plr.y > y)      yd++;
				else if(w->plr.y < y) yd--;
				
				if(w->t[yd][xd].flags & TFLAG_INTRINSIC_LIGHT)
					change = tracevision(x, y, w->plr.x, w->plr.y);
			}
		}
		else
		{
			if(is_in_range && tracevision(x, y, w->plr.x, w->plr.y))
				change = 1;
		}
		if(change)
		{
			w->t[y][x].flags |= (TFLAG_LIT | TFLAG_EXPLORED);
			draw_tile(x, y);
		}
	}
}
//}}}
//{{{
ushort tracevision(sshort x1, sshort y1, sshort x2, sshort y2)
/*
 * Check whether the straight path from (x1,y1) to (x2,y2) is blocked (but
 * with a special case when (x1,y1) is an opaque tile itself).
 *
 * This is the most speed-critical function in the entire project. Kluge with
 * care. Also, pay attention to the sizes of struct tile and struct tiledesc
 * - if they're not powers of 2, this function will be much slower than it
 * should be due to extra address calculations.
 */
{
	sshort xdist, ydist;
	sshort xi, yi;
	sshort xdir, ydir;
	
	xdir=(x1<x2)?1 : (x1==x2)?0:-1;
	ydir=(y1<y2)?1 : (y1==y2)?0:-1;
	
	// Hack: If tracing to a non-transparent tile (such as a wall), move one
	// tile towards the player. This solves the corner and edge problems
	// without introducing any noticeable false positives
	if( !w->tiledescs[w->t[y1][x1].type].transparent ) {
		x1 += xdir;
		y1 += ydir;
	}
	
	xdist=abs(x1-x2); ydist=abs(y1-y2);
	xi=(ydist/2); yi=(xdist/2);
	
	while(x1!=x2 || y1!=y2)
	{
		if( !w->tiledescs[w->t[y1][x1].type].transparent )
			return 0;
		
		xi+=xdist; yi+=ydist;
		if(xi>=ydist) {
			x1 += xdir;
			xi -= ydist;
		}
		if(yi>=xdist) {
			y1 += ydir;
			yi -= xdist;
		}
	}
	
	return 1;
}
//}}}
//{{{
void illuminate(sint x, sint y, sint radiussquared)
/*
 * Mark all tiles within sqrt(radiussquared) of (x,y) which can see (x,y) as
 * intrinsically lit. Used for lighting up rooms.
 */
{
	int i, j;
	
	for(i=0; i<MAPSIZE_Y; i++)
	for(j=0; j<MAPSIZE_X; j++)
	{
		if( (x-j)*(x-j)+(y-i)*(y-i) <= radiussquared && tracevision(j,i,x,y) )
			w->t[i][j].flags |= TFLAG_INTRINSIC_LIGHT;
	}
}
//}}}

