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
// rand.c
/// @file
/// @brief Random number generator

#include "crogue.h"

//{{{
// High-byte is maximum
// Low-byte is minimum
uchar xrandom(uint range)
{
	return nrandom(range>>8, range & 0x00FF);
}
//}}}
//{{{
uchar rrandom(range r) 
{
	return lrand()%(r.max-r.min+1) + r.min;
}
//}}}
//{{{
sint nrandom(sint max, sint min)
{
	return lrand()%(max-min+1) + min;
}
//}}}

static unsigned long seed;

//{{{
// Seed the random number generator
void lsrand(unsigned long s)
{
	seed = s;
}
//}}}
//{{{
// Generate a random number in [0,LONG_MAX]
unsigned long lrand(void)
{
	seed *= 1103515245;
	seed += 12345;
	return (seed ^ (seed>>16)) & 0x7FFFFFFF;
}
//}}}
//{{{
// Given a random number, use it to make the random number generator more
// random. The number given does not have to be perfectly (or even mostly)
// random; anything unpredictable will do.
void feed_entropy(unsigned long entropy)
{
	seed += entropy;
	lrand();
}
//}}}

