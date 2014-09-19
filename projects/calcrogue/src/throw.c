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
// throw.c
/// @file
/// @brief Throwing projectiles and other items

#include "crogue.h"

static void throw_item(int which_item, direction dir);
static void thrown_item_effect(item *itm, int monst);

//{{{
void throw_item_from_menu(void)
{
	int which;
	
	if(filter_matches(filter_none)==0)
	{
		message(gettext("You don't have anything to throw."));
		return;
	}
	message(gettext("Throw what?"));
	which = pick_item(filter_none);
	
	if(which<0)
	{
		message(gettext("Never mind."));
		return;
	}
	
	throw_item(which, DIRECTION(0,0));
}
//}}}
//{{{
void throw_readied_item(void)
{
	fire_dir(0, 0);
}
//}}}
//{{{
void fire_dir(int x, int y)
{
	// FIXME: This should be handled through player_equipment()
	int ii;
	for(ii=0; ii<PLRINVSIZE; ii++)
		if(INVENTORY(ii).type && ITEMDESC(INVENTORY(ii)).spot == 1 && INVENTORY(ii).flags & ITEMFLAG_EQUIPPED)
		{
			throw_item(ii, DIRECTION(x,y));
			return;
		}
	
	message(gettext("You don't have any ammunition readied."));
}
//}}}

//{{{
static void throw_item(int which_item, direction dir)
{
	coord landing;
	sint monst;
	uint finishlanding = 1;
	sint is_launched;
	item projectile;
	sint tohitmod = 0;
	sint range = 8;
	sint damage;
	
	projectile = split_item(which_item);
	
	if( get_item_property(&projectile, PROPERTY_LAUNCHER) )
		is_launched = 1;
	else
		is_launched = 0;
	
	if( is_launched &&
	    !( w->plr.extrinsic[STAT_MISSILES] & get_item_property(&projectile, PROPERTY_LAUNCHER)->launcher ) )
	{
		if(w->plr.extrinsic[STAT_MISSILES])
			message(gettext("You need an appropriate launcher for that ammo."));
		else
			message(gettext("You need a launcher for that ammo."));
		goto abort;
	}
	if(projectile.flags & ITEMFLAG_EQUIPPED && ITEMDESC(projectile).spot != 1)
	{
		message(gettext("You can't throw something you're wearing."));
		goto abort;
	}
	apply_all_item_properties(&projectile, PROPERTY_THROWN_EFFECT); // Applies ammo effects on player
	tohitmod = w->plr.extrinsic[STAT_MISSILE_TOHIT] + weapon_skill_attack();
	damage = nrandom(w->plr.extrinsic[STAT_MISSILEDAM_MAX], w->plr.extrinsic[STAT_MISSILEDAM_MIN]);
	damage += projectile.plus + weapon_skill_damage();
	tohitmod += projectile.plus;
	tohitmod += w->plr.level*2;
	
	projectile.flags &= ~ITEMFLAG_EQUIPPED; // Unmark ammo as 'readied'
	
	if(dir.x==0 && dir.y==0)
		prompt_dir(gettext("In what direction?"), &dir);
	
	landing = trace_path(w->plr.x, w->plr.y, dir, range);
	
	// If a wall was hit, back up one tile to find the contact point
	if(!w->tiledescs[w->t[landing.y][landing.x].type].passable) {
		landing.x -= dir.x;
		landing.y -= dir.y;
	}
	
	// If a monster was hit, hurt it and (maybe) do magical effects
	monst = monstbytile(landing.x, landing.y);
	if(monst != -1) {
#ifdef DEBUG_BALANCE
	if(w->debug_mode)
		message("(%i,%i)v[%i,%i]{%i/%i}",
			tohitmod, damage,
			MDESC(monst)->dv, MDESC(monst)->pv,
			w->m[monst].hps, MDESC(monst)->hps_max);
#endif
		if(plr_attack_monster_roll(monst, tohitmod)) {
			message(gettext("The %s hits the %s."), shortitemname(&projectile), monstname(monst));
			monst_anger(monst);
			if(!isNull(w->m[monst].type))
				monst_takedamage(monst, damage, 0);
			thrown_item_effect(&projectile, monst);
		} else {
			message(gettext("The %s dodges the %s."), monstname(monst), shortitemname(&projectile));
			monst_anger(monst);
		}
	}
	
	if( finishlanding ) {
		if( ITEMDESC(projectile).flags & ITEMFLAG_FRAGILE ) {
			message(gettext("The %s shatters!"), shortitemname(&projectile));
		} else if( (ITEMDESC(projectile).flags & ITEMFLAG_BREAKABLE) && RANGE(max(2, 3 + 2*projectile.plus), 1) == 1 ) {
			message(gettext("The %s breaks."), shortitemname(&projectile));
		} else {
			place_item(&projectile, landing.x, landing.y);
			draw_tile(landing.x, landing.y);
		}
	}
	
	award_mark();
	update_player(); // Undoes ammo effects
	timepass(1);
	return;

abort:
	merge_item(which_item, projectile);
}
//}}}
//{{{
static void thrown_item_effect(item *itm, int monst)
{
	const item_property_desc *f;
	f = get_item_property(itm, PROPERTY_THROW_HIT_EFFECT);
	if(!f)
		return;
	call_throwhitfunc(f->function, monst);
}
//}}}


