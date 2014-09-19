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
// magic.c
/// @file
/// @brief Helper functions for spells and magical effects

#include "crogue.h"

//{{{
ushort attack_tile(uint x, uint y, const char *str, const char *death_message, uint damage)
{
	int monst = monstbytile(x, y);
	if(monst >= 0)
	{
		message(str, retprintf(gettext("the %s"), monstname(monst)));
		monst_anger(monst);
		monst_takedamage(monst, damage, 0);
		return 1;
	} else if(x == w->plr.x && y == w->plr.y)
	{
		message(str, gettext("you"));
		plr_takedamage(damage, death_message);
		return 1;
	} else
	{
		return 0;
	}
}
//}}}
//{{{
ushort identify(void)
{
	int which;
	
	if(filter_matches(filter_unidentified)==0)
	{
		message(gettext("You have nothing to identify."));
		return 0;
	}
	
	message(gettext("Identify what?"));
	which = pick_item(filter_unidentified);
	
	if(which<0)
	{
		message(gettext("You waste the spell."));
		return 0;
	}
	
	identify_type( INVENTORY(which).type );
	INVENTORY(which).flags |= ITEMFLAG_IDENTIFIED;
	
	message("%s.", itemname(&INVENTORY(which)));
	return 1;
}
//}}}

//{{{
static const stat_timer_desc entangle_timer = {
		TIMER_STAT,
		{STATMOD_OR, STAT_FLAGS, STAT_FLAG_ENTANGLED},
		gettext("You break free of the webs.")
	};
//}}}
//{{{
void entangle(uint duration)
{
	add_timer((timer_desc*)&entangle_timer, duration);
}
//}}}
//{{{
static const stat_timer_desc confuse_timer = {
		TIMER_STAT,
		{STATMOD_OR, STAT_FLAGS, STAT_FLAG_CONFUSED},
		gettext("You are no longer confused.")
	};
//}}}
//{{{
void confuse(uint duration)
{
	add_timer((timer_desc*)&confuse_timer, duration);
}
//}}}
//{{{
static const stat_timer_desc hallucinate_timer = {
		TIMER_STAT,
		{STATMOD_OR, STAT_FLAGS, STAT_FLAG_HALLUCINATING},
		gettext("You stop hallucinating.")
	};
//}}}
//{{{
void hallucinate(uint duration)
{
	add_timer((timer_desc*)&hallucinate_timer, duration);
}
//}}}
//{{{
static const stat_timer_desc paralyzation_timer = {
		TIMER_STAT,
		{STATMOD_OR, STAT_FLAGS, STAT_FLAG_PARALYZED},
		gettext("You can move again.")
	};
//}}}
//{{{
void paralyze(uint duration)
{
	add_timer((timer_desc*)&paralyzation_timer, duration);
}
//}}}
//{{{
static const stat_timer_desc darkness_timer = {
		TIMER_STAT,
		{STATMOD_ADD, STAT_LIGHTRADIUS, -4},
		gettext("The area around you is bright again.")
	};
//}}}
//{{{
void engulf_in_darkness(sint duration)
{
	add_timer((timer_desc*)&darkness_timer, duration);
}
//}}}

static void potion_numbness_effect(void);
static void potion_numbness_expire(void);
//{{{
static const func_timer_desc numbness_timer = {
		TIMER_FUNC,
		&potion_numbness_effect,
		&potion_numbness_expire
	};
//}}}
//{{{
void numb(uint duration)
{
	add_timer((timer_desc*)&numbness_timer, duration);
}
//}}}
//{{{
static void potion_numbness_effect(void)
{
	const stat_mod_desc numbness_effect = {STATMOD_OR, STAT_FLAGS, STAT_FLAG_NUMB};
	const stat_mod_desc dexterity_effect = {STATMOD_ADD, STAT_DEXTERITY, -5};
	apply_stat_mod_desc(NULL, &numbness_effect, 0);
	apply_stat_mod_desc(NULL, &dexterity_effect, 0);
}
//}}}
//{{{
static void potion_numbness_expire(void)
{
	sint hp_deficit;
	uint stat_drain;
	
	if(w->plr.hps <= 0)
	{
		message(gettext(
			"You feel excrutiating pain as the feeling returns to your body. "
			"You have seriously abused your system."));
		for(hp_deficit = -w->plr.hps; hp_deficit>0; hp_deficit -= 12)
		{
			if(RANGE(1, 0))
				stat_drain = STAT_STRENGTH;
			else
				stat_drain = STAT_TOUGHNESS;
			
			w->plr.intrinsic[stat_drain] --;
			if(w->plr.intrinsic[stat_drain] <= 0)
				die(gettext("Died of system shock."));
		}
		w->plr.hps = 1;
		update_player();
	}
	else if(w->plr.hps * 4 < w->plr.hps_max)
		message(gettext("You feel a sharp pain as the feeling returns to your body."));
	else
		message(gettext("Feeling returns to your body."));
}
//}}}

//{{{
static const stat_timer_desc detect_invis_timer = {
		TIMER_STAT,
		{STATMOD_OR, STAT_FLAGS, STAT_FLAG_SEEINVIS},
		gettext("Your vision darkens.")
	};
//}}}
//{{{
void detect_invisible(uint duration)
{
	add_timer((timer_desc*)&detect_invis_timer, duration);
}
//}}}

//{{{
void removecurses(void)
{
	int which;
	
	if(filter_matches(filter_cursed)==0)
	{
		message(gettext("You have nothing to uncurse."));
		return;
	}
	message(gettext("Uncurse which item?"));
	which = pick_item(filter_cursed);
	
	if(which<0)
	{
		message(gettext("You waste the scroll."));
		return;
	}
	
	if(INVENTORY(which).flags & ITEMFLAG_CURSED)
	{
		message(gettext("It glows for a moment."));
		INVENTORY(which).flags &= ~ITEMFLAG_CURSED;
	} else {
		message(gettext("Nothing happens."));
	}
}
//}}}
//{{{
int enchant_item(void)
{
	int which;
	if(filter_matches(filter_enchantable)==0)
	{
		message(gettext("You have nothing to enchant."));
		return 0;
	}
	message(gettext("Enchant what?"));
	which = pick_item(filter_enchantable);
	
	if(which<0)
	{
		message(gettext("You waste the scroll."));
		return 0;
	}
	
	if(ITEMDESC(INVENTORY(which)).flags & ITEMFLAG_CHARGED)
	{
		message(gettext("The %s glows in white light."), itemname(&INVENTORY(which)));
		INVENTORY(which).stacksize += 15;
	}
	else if(ITEMDESC(INVENTORY(which)).flags & ITEMFLAG_PLUS)
	{
		if(INVENTORY(which).plus < 2)
			INVENTORY(which).plus += RANGE(2,1);
		else if(INVENTORY(which).plus < 4)
			INVENTORY(which).plus ++;
		else if(INVENTORY(which).plus < 6) {
			if( RANGE(2,1) == 1 )
				INVENTORY(which).plus ++;
			else
				goto failed_enchant;
		} else
			goto failed_enchant;
		message(gettext("The %s glows in blue light."), itemname(&INVENTORY(which)));
	} else {
failed_enchant:
		message(gettext("Nothing happens."));
	}
	update_player();
	return 1;
}
//}}}

