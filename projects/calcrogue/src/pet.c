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
// pet.c
/// @file
/// @brief AI for friendly creatures and monster-vs-monster combat.

#include "crogue.h"

//{{{
void pet_move(int pet)
{
	int xpos, ypos;
	int xi, yi;
	int target;
	xpos = w->m[pet].x;
	ypos = w->m[pet].y;
	
	// If a monster is adjacent, attack it
	for(yi = ypos-1; yi <= ypos+1; yi++)
	for(xi = xpos-1; xi <= xpos+1; xi++)
	{
		if(xi == xpos && yi == ypos) continue;
		
		target = monstbytile(xi, yi);
		if(target == -1) continue;
		if(monst_is_peaceful(target)) continue;
		
		monst_swing_monst(pet, target, MTRIGGER_NORMAL);
		return;
	}
	
	// If far from the player, follow him
	if( distancesquare(w->plr.x, w->plr.y, xpos, ypos) > 4 )
	{
		monstmovetowardsplayer(pet);
		return;
	}
	
	// Otherwise, move randomly
	monstmoverandomly(pet, 0);
}
//}}}

//{{{
void monst_swing_monst(int attacker, int target, int trigger)
{
	int i;
	int ret=0;
	
	for(i=0; i<MDESC(attacker)->numattacks; i++)
	{
		if(MATTACK(attacker, i).trigger == trigger)
		{
			ret = 1;
			if(monst_hit_monst(i, attacker, target))
				monst_attack_monst(i, attacker, target);
			else
			{
				if(player_can_see(attacker) && player_can_see(target))
					message(gettext("The %s misses the %s!"), monstname(attacker), monstname(target));
				else if(player_can_see(attacker))
					message(gettext("The %s misses something!"), monstname(attacker));
				else if(player_can_see(target))
					message(gettext("The %s evades something!"), monstname(target));
				else
					message(gettext("You hear the sound of combat."));
			}
		}
		
		if(isNull(w->m[attacker].type) || isNull(w->m[target].type))
			return;
	}
}
//}}}
//{{{
void monst_attack_monst(int attacknum, int attacker, int target)
{
	int damage;
	monstattack *attacks;
	
	if(attacker == target) // sanity check
		return;
	
	attacks = MDESC(attacker)->attacks;
	damage = rrandom( attacks[attacknum].damage );
	
	monst_anger(target);
	call_attackfunc( attacks[attacknum].type, attacker, damage, target );
}
//}}}

//{{{
// If able to attack a pet belonging to the player, do so
ushort monst_battle_pet(ushort monst)
{
	int xpos, ypos;
	int xi, yi;
	int target;
	xpos = w->m[monst].x;
	ypos = w->m[monst].y;
	
	for(yi = ypos-1; yi <= ypos+1; yi++)
	for(xi = xpos-1; xi <= xpos+1; xi++)
	{
		target = monstbytile(xi, yi);
		if(target >= 0)
		{
			if(monst_is_pet(target))
			{
				monst_swing_monst(monst, target, MTRIGGER_NORMAL);
				return 1;
			}
		}
	}
	return 0;
}
//}}}

