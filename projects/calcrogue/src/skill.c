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
// skill.c
/// @file
/// @brief Weapon skills

#include "crogue.h"
#define NUM_SKILL_LEVELS 10

static int marks_to_advance(int skill_num);

typedef struct skill_desc
{
	const char *skill_name;
} skill_desc;

typedef struct skill_level_desc
{
	schar hit_effect;
	schar damage_effect;
} skill_level_desc;

static skill_desc skill_descs[NUM_SKILLS+1] = {
		{ ""                   },
		{ gettext("bows")      },
		{ gettext("crossbows") },
		{ gettext("clubs")     },
		{ gettext("daggers")   },
		{ gettext("staves")    },
		{ gettext("maces")     },
		{ gettext("swords")    },
		{ gettext("axes")      }
	};

static skill_level_desc skill_levels[NUM_SKILL_LEVELS] = {
		{ -3,  -3  },
		{ -1,  -1, },
		{ 0,   0,  },
		{ 1,   0,  },
		{ 2,   0,  },
		{ 3,   1,  },
		{ 5,   1,  },
		{ 7,   2,  },
		{ 9,   2,  },
		{ 11,  3,  },
	};

static int advancement_marks[NUM_SKILL_LEVELS] = {
		2,
		3,
		4,
		6,
		9,
		12,
		15,
		18,
		22,
		30
	};

//{{{
static int marks_to_advance(int skill_num)
{
	int level = w->plr.skill[skill_num];
	
	return advancement_marks[level] * w->playerclasses[w->plr.class].weapon_skill_factor;
}
//}}}
//{{{
// Note: The currently-wielded weapon is special, so this must be called AFTER
// inventory initialization
void init_skills(void)
{
	int i;
	for(i=0; i<=NUM_SKILLS; i++)
	{
		w->plr.skill_marks[i] = 0;
		w->plr.skill[i] = 0;
	}
	w->plr.skill[w->plr.extrinsic[STAT_WEAPON_TYPE]] = 2;
	for(i=0; i<=NUM_SKILLS; i++)
		w->plr.skill[i] += w->playerclasses[w->plr.class].weapon_skill_start;
}
//}}}
//{{{
void award_mark(void)
{
	int skill_num = w->plr.extrinsic[STAT_WEAPON_TYPE];
	
	if(!skill_num) return;
	
	w->plr.skill_marks[skill_num]++;
	
	if(w->plr.skill_marks[skill_num] > marks_to_advance(skill_num) && w->plr.skill[skill_num] < NUM_SKILL_LEVELS)
	{
		message(gettext("You improve your skill with %s to level %i."), skill_descs[skill_num], ++w->plr.skill[skill_num]);
		w->plr.skill_marks[skill_num] = 0;
		update_player();
	}
}
//}}}
//{{{
sint weapon_skill_attack(void)
{
	int skill_used = w->plr.extrinsic[STAT_WEAPON_TYPE];
	return skill_levels[w->plr.skill[skill_used]].hit_effect;
}
//}}}
//{{{
sint weapon_skill_damage(void)
{
	int skill_used = w->plr.extrinsic[STAT_WEAPON_TYPE];
	return skill_levels[w->plr.skill[skill_used]].damage_effect;
}
//}}}
//{{{
void print_skills(char *buf)
{
	int i;
	for(i=1; i<=NUM_SKILLS; i++)
		catprintf(buf, "%s\t%i\n", skill_descs[i].skill_name, w->plr.skill[i]);
}
//}}}

