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
// facing.c
/// @file
/// @brief Implements parts of the Implicit Facing rules.
/// See notes/facing.txt for details on what implicit facing is.

#include "crogue.h"

//{{{
static sint sign(sint v)
{
	if(v<0) return -1;
	else if(v>0) return 1;
	else return 0;
}
//}}}
//{{{
static schar facings[3][3] =
	{
		{ 3, 2, 1 },
		{ 4, 8, 0 },
		{ 5, 6, 7 }
	};
//}}}
//{{{
sint facing(sint x, sint y)
{
	x = sign(x) + 1;
	y = sign(y) + 1;
	return facings[y][x];
}
//}}}
//{{{
sint facing_towards(sint x, sint y)
{
	return facing((sint)x - w->plr.x, (sint)y - w->plr.y);
}
//}}}
//{{{
sint facing_opposite(sint x, sint y)
{
	return (facing_towards(x, y) + 4) % 8;
}
//}}}
//{{{
sint player_facing(void)
{
	int xi, yi, n=0, f=0;
	
	if(w->plr.facing_forced != -1)
		return w->plr.facing_forced;
	
	for(yi=w->plr.y-1; yi<=w->plr.y+1; yi++)
	for(xi=w->plr.x-1; xi<=w->plr.x+1; xi++)
	{
		if(monstbytile(xi, yi) != -1)
			n++, f=facing_towards(xi, yi);
	}
	if(n==1) return f;
	else     return w->plr.facing;
}
//}}}
//{{{
uint delta_facing(sint facing_one, sint facing_two)
{
	sint df = abs(facing_one - facing_two);
	return min(df, 8-df);
}
//}}}
