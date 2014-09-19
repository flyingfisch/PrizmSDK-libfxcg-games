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
// Construct.c
/// @file
/// @brief A workaround for compilers without cast-constructors.
///
/// Construction is done with a cast constructor in TIGCC/GCC, or with these
/// functions (much slower) anywhere else. Speed only matters on the calc, so
/// nothing is lost this way.

#include "crogue.h"

#ifndef CASTCONSTRUCT

//{{{
coord construct_coord(uchar x, uchar y)
{
	coord c;
	
	c.x = x; c.y=y;
	return c;
}
//}}}
//{{{
rect construct_rect(uchar x, uchar y, uchar w, uchar h)
{
	rect r;
	r.topleft.x = x;
	r.topleft.y=y;
	r.extent.x = w;
	r.extent.y = h;
	return r;
}
//}}}
//{{{
direction construct_direction(schar x, schar y)
{
	direction d;
	
	d.x = x;
	d.y = y;
	
	return d;
}
//}}}
//{{{
range construct_range(uchar max, uchar min)
{
	range r;
	
	r.max = max;
	r.min = min;
	
	return r;
}
//}}}

#endif
