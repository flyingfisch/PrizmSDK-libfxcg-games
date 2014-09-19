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
// spell.c
/// @file
/// @brief Handles learning from spellbooks. 
///
/// Note that the spells themselves are in the DLL and in the data file.

#include "crogue.h"
#include "export.h"

#define SPELL_TIME 4000
	///< Number of turns before a spell is forgotten if not cast
#define SPELL_INCREMENT 15
	///< Number of turns added for each mana spent on this spell
#define MAX_SPELL_TIME 9000
	///< Maximum number of turns remaining a spell can reach from increments

static int numSpellsKnown(void);
static int spellByIndex(int n);
static void study_spell(int spell, int type);
static void learn_spell(int spell);
static void refresh_spell(int spell);
static void spellbookString(int n, char *buf);

static const struct spelldesc **spell_list;

//{{{
sint study_book(ushort num_spells, ushort type, va_list spells) 
{
	const struct spelldesc *spell_data[16];
	int spellnum;
	int i;
	spell_list = spell_data;
	
	if(w->playerclasses[w->plr.class].spellcasting[type] == 0)
	{
		if(type == SPELLTYPE_ARCANE)
			message(gettext("You don't understand the language."));
		else if(type == SPELLTYPE_DIVINE)
			message(gettext("This book is filled with religious mumbo-jumbo."));
		else if(type == SPELLTYPE_NECROMANCY)
			message(gettext("This book is filled with evil rituals."));
		return 0;
	}
	

	for(i=0; i<num_spells; i++)
	{
		spellnum = va_arg(spells, ulong);
		spell_data[i] = &w->spelldescs[spellnum];
	}
	
	message(gettext("Study which spell?"));
	setTabStops(spell_menu_tabs);
	spellnum = UI_Menu_Pick(learn_spell_rect, num_spells, &spellbookString);
	full_redraw();
	
	if(spellnum < 0)
	{
		message(gettext("Never mind."));
		return 0;
	}
	else
	{
		study_spell(spell_data[spellnum]->num, type);
		return 1;
	}
}
//}}}

//{{{
static void study_spell(int spell, int type)
{
	int i;
	int read_success;
	int slots_used = 1;
	
	if( w->spelldescs[spell].difficulty >
		(w->playerclasses[w->plr.class].spellcasting[type]
		+ w->plr.extrinsic[STAT_WILLPOWER]/4) * w->plr.level / 3
		||
		w->spelldescs[spell].pp_cost > w->plr.pps_max
		)
			read_success = 0;
	else
			read_success = 1;

	if(w->plr.spellknowledge[spell] > (slong)(w->time) + SPELL_TIME + 500)
	{
		message(gettext("You know that spell practically by heart."));
		return;
	}
	else if(w->plr.spellknowledge[spell] > (slong)(w->time) + (SPELL_TIME/2))
	{
		message(gettext("You know that spell already."));
		return;
	}
	else if(w->plr.spellknowledge[spell] >= 0)
	{
		read_success = 1;
		slots_used = 0;
	}
	
	if( read_success == 0 )
	{
		message(gettext("That spell is beyond your ability."));
		return;
	}
	
	if( slots_used &&
	    numSpellsKnown() >= (w->playerclasses[w->plr.class].spellcasting[type]
	                         * (w->plr.level+1))/20 )
	{
		if(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_HALLUCINATING)
			message(gettext("Sorry, Mr. Osbourne, but my brain is full."));
		else
			message(gettext("You can't learn any more spells yet."));
		return;
	}

	w->interrupt = 0;
	for(i=0; i<w->spelldescs[spell].difficulty; i++)
	{
		timepass(1);
		if(w->interrupt)
		{
			message(gettext("Your concentration is broken."));
			return;
		}
	}
	
	if(slots_used)
		learn_spell(spell);
	else
		refresh_spell(spell);
}
//}}}
//{{{
static void refresh_spell(int spell)
{
	w->plr.spellknowledge[spell] = (slong)(w->time + SPELL_TIME);
	message(gettext("You refresh your memory of the '%s' spell."),
		deref_file_ptr(w->spelldescs[spell].name));
}
//}}}
//{{{
static void learn_spell(int spell)
{
	w->plr.spellknowledge[spell] = (slong)(w->time + SPELL_TIME);
	message(gettext("You learn the '%s' spell."),
		deref_file_ptr(w->spelldescs[spell].name));
	call_genericfunc(w->spelldescs[spell].learn_func);
}
//}}}

//{{{
void init_spells(void)
{
	int i;
	
	w->plr.spellknowledge = debug_malloc( sizeof(slong)*w->desc.numspells );
	
	for(i=0; i<w->desc.numspells; i++)
		w->plr.spellknowledge[i] = -1;
}
//}}}
//{{{
static int numSpellsKnown(void)
{
	int i, n=0;
	
	for(i=0; i<w->desc.numspells; i++)
		if(w->plr.spellknowledge[i] != -1) n++;
	
	return n;
}
//}}}
//{{{
static int spellByIndex(int n)
{
	int i;
	
	for(i=0; n>=0; i++) {
		if(w->plr.spellknowledge[i] != -1)
			n--;
	}
	
	return i-1;
}
//}}}
//{{{
static void spellString(int n, char *buf)
{
	const struct spelldesc *s;
	const char *spell_status;
	int spell_index = spellByIndex(n);
	
	s = &w->spelldescs[spell_index];
	
	if(w->plr.spellknowledge[spell_index] > w->time + 800)
		spell_status = gettext("memorized");
	else if(w->plr.spellknowledge[spell_index] >= w->time)
		spell_status = gettext("known");
	else
		spell_status = gettext("forgotten");
	
#ifdef IS_CALCULATOR
	sprintf(buf, "%i\t%s\t"   "%i " gettext("pp") "\t"   "%s", (n+1)%10, (const char*)deref_file_ptr(s->name), s->pp_cost, spell_status);
#else
	sprintf(buf, "%s\t"   "%i " gettext("pp") "\t"   "%s", (const char*)deref_file_ptr(s->name), s->pp_cost, spell_status);
#endif
}
//}}}
//{{{
static void spellbookString(int n, char *buf)
{
	const struct spelldesc *s;
	
	s = spell_list[n];
	
#ifdef IS_CALCULATOR
	sprintf(buf, "%i\t%s\t%i " gettext("pp") "\t", (n+1)%10, (const char*)deref_file_ptr(s->name), s->pp_cost);
#else
	sprintf(buf, "%s\t%i " gettext("pp") "\t", (const char*)deref_file_ptr(s->name), s->pp_cost);
#endif
}
//}}}

//{{{
void cast_spell(void)
{
	int which;
	
	if(!numSpellsKnown())
	{
		message(gettext("You don\'t know any spells."));
		return;
	}
	
	if(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_CONFUSED)
	{
		message(gettext("You are too confused to cast spells!"));
		return;
	}
	
	setTabStops(spell_menu_tabs);
	which = UI_Menu_Pick( cast_spell_rect, numSpellsKnown(), &spellString );
	full_redraw();
	
	if(which==-1)
	{
		message(gettext("Never mind."));
		return;
	}
	
	which = spellByIndex(which);
	
	if(w->plr.spellknowledge[which] < w->time)
	{
		message(gettext("You've forgotten that spell.\n"));
		return;
	}
	
	if(w->plr.pps < w->spelldescs[which].pp_cost)
	{
		message(gettext("You don\'t have enough power to cast that spell."));
		return;
	}
	w->plr.spellknowledge[which]+=SPELL_INCREMENT*w->spelldescs[which].pp_cost;
	if(w->plr.spellknowledge[which] > w->time+MAX_SPELL_TIME)
		w->plr.spellknowledge[which] = w->time+MAX_SPELL_TIME;
	w->plr.pps -= w->spelldescs[which].pp_cost;
	
	call_genericfunc(w->spelldescs[which].function);
	
	timepass(1);
}
//}}}

