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
// ai.c
/// @file
/// @brief Monster AI

#include "crogue.h"

//{{{
static int count_adjacent_tiles(int x, int y, int type)
{
	sint xi, yi;
	uint count=0;
	
	for(xi=x-1; xi<=x+1; xi++)
	for(yi=y-1; yi<=y+1; yi++)
	{
		if(xi<0 || yi<0 || xi>=MAPSIZE_X || yi>=MAPSIZE_Y)
			continue;
		if(w->t[yi][xi].type == type)
			count++;
	}
	return count;
}
//}}}

//{{{
void monstmove(sint monst)
{
	int flags = w->m[monst].flags;
	
	if( MDESC(monst)->flags & MONSTFLAG_REGENERATE &&
	    w->m[monst].hps < MDESC(monst)->hps_max)
	{
		w->m[monst].hps += 1;
		
		if(player_can_see(monst))
			message( gettext("The %s regenerates!"), monstname(monst));
	}
	
	if( MDESC(monst)->power_max > w->m[monst].power ) {
		w->m[monst].power ++;
	}
	
	if(flags & (MONST_PARALYZED|MONST_CONFUSED|MONST_PET|MONST_SCARED))
	{
		if(flags & MONST_PARALYZED) {
			if(RANGE(20,1)==1)
				w->m[monst].flags &= ~MONST_PARALYZED;
		} else if(flags & MONST_CONFUSED) {
			monstmoverandomly(monst, 1);
		}
		else if(flags & MONST_PET) {
			pet_move(monst);
		}
		else if(flags & MONST_SCARED) {
			if(!monst_detect_player(monst) && RANGE(5,1)==1)
				w->m[monst].flags &= ~MONST_SCARED;
			if(w->m[monst].hps*2 > MDESC(monst)->hps_max && RANGE(15,1)==1)
			{
				if(player_can_see(monst))
					message(gettext("The %s regains its courage."),
					    monstname(monst));
				w->m[monst].flags &= ~MONST_SCARED;
			}
			if(RANGE(4,1)==1)
				w->m[monst].hps++;
			monstmoveawayfromplayer(monst);
		}
	}
	else if(flags & MONST_ACTIVE)
	{
		if(flags & MONST_CUST_WHEN_ACTIVE) {
			call_aifunc( MDESC(monst)->ai_active, monst );
		} else if(flags & (MONST_PEACEFUL|MONST_FRIENDLY)) {
			if(flags & MONST_WANDER_WHEN_IDLE)
				monstmoverandomly(monst, 0);
			else
				;
		} else {
			if(monstcanmove(monst, w->plr.x, w->plr.y))
				monstmovetowardsplayer(monst);
			else if(monst_is_aligned_with_plr(monst) &&
					  monst_attack_type(MTRIGGER_RANGED, monst, 1) )
				;
			else if(monst_battle_pet(monst))
				;
			else if(!monst_detect_player(monst))
				w->m[monst].flags &= ~MONST_ACTIVE;
			else if(monst_attack_type(MTRIGGER_SEEN, monst, 1) )
				;
			else
				monstmovetowardsplayer(monst);
		}
	}
	else
	{
		if(distancesquare(w->m[monst].x, w->m[monst].y, w->plr.x, w->plr.y)
		   > 400)
			return;
		else if(flags & MONST_CUST_WHEN_IDLE) {
			call_aifunc( MDESC(monst)->ai_idle, monst );
		} else {
			if(monst_detect_player(monst))
				w->m[monst].flags |= MONST_ACTIVE;
			if(flags & MONST_WANDER_WHEN_IDLE) {
				if(flags & (MONST_PEACEFUL|MONST_FRIENDLY))
					monstmoverandomly(monst, 0);
				else
					monstmoverandomly(monst, 1);
			}
		}
	}
}
//}}}
//{{{
//
// 1 Randomly pick a move; if it's a web, move onto it.
// 2 (50% chance) Pick the adjacent floor tile with the most webs bordering
//   it, and spin a web there.
//
void webspinner_build(ushort monst, ushort tile_web, ushort tile_floor)
{
	int x, y, temp;
	int xi, yi;
	int best = -1;
	int best_x = w->m[monst].x;
	int best_y = w->m[monst].y;
	
	x = w->m[monst].x + RANGE(2,0) - 1;
	y = w->m[monst].y + RANGE(2,0) - 1;
	
	if(w->t[y][x].type == tile_web && monstcanmove(monst, x, y))
	{
		monstmoveto(monst, x, y);
		return;
	}
	
	if(RANGE(1,0))
	{
		for(yi = w->m[monst].y-1; yi <= w->m[monst].y+1; yi++)
			for(xi = w->m[monst].x-1; xi <= w->m[monst].x+1; xi++)
			{
				if(w->t[yi][xi].type != tile_floor)
					continue;
				if(!monstcanmove(monst, xi, yi))
					continue;
				temp = count_adjacent_tiles(xi, yi, tile_web);
				if(temp > best)
				{
					best = temp;
					best_x = xi;
					best_y = yi;
				}
			}
		monstmoveto(monst, best_x, best_y);
		w->t[best_y][best_x].type = tile_web;
	}
}
//}}}
//{{{
//
// (a) minimize the number of adjacent non-wall, non-web tiles
// (b) move as close to the player as possible
//
void webspinner_cower(ushort monst, ushort tile_web, ushort tile_wall)
{
	int best_distance = 1000, best_webbing = 0, temp1, temp2;
	int xi, yi;
	int target;
	int best_x = w->m[monst].x;
	int best_y = w->m[monst].y;
	
			// Be more aggressive when the player is stuck in webs
	if(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_ENTANGLED) {
		monstmovetowardsplayer(monst);
		return;
	}
	
	for(yi = w->m[monst].y-1; yi <= w->m[monst].y+1; yi++)
	for(xi = w->m[monst].x-1; xi <= w->m[monst].x+1; xi++)
	{
		if(w->t[yi][xi].type != tile_web)
			continue;
		target = monstbytile(xi, yi);
		if(target>=0 && target != monst)
			continue;
		temp1 = distancesquare(xi, yi, w->plr.x, w->plr.y);
		// HACKHACK: Doesn't handle non-diggable walls, doors, etc.
		temp2 = count_adjacent_tiles(xi, yi, tile_web)
			  + count_adjacent_tiles(xi, yi, tile_wall);
		if( temp2>best_webbing ||
			(temp1<best_distance && temp2==best_webbing)
		  )
		{
			best_distance = temp1;
			best_webbing = temp2;
			best_x = xi;
			best_y = yi;
		}
	}
	monstmoveto(monst, best_x, best_y);
}
//}}}
//{{{
void monst_chat(ushort monst)
{
	if(monstcanmove(monst, w->plr.x, w->plr.y))
		call_chatfunc(MDESC(monst)->chatfunc, monst);
}
//}}}

