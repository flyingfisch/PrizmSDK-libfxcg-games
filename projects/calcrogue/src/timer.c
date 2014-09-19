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
// timer.c
/// @file
/// @brief Controls timed effects

#include "crogue.h"

//{{{
void init_timers(void)
{
	int i;
	for(i=0; i<NUM_STATMOD_TIMERS; i++)
		w->plr.stat_timers[i].desc = NULL;
}
//}}}
//{{{
void apply_timers(void)
{
	int i;
	for(i=0; i<NUM_STATMOD_TIMERS; i++)
		if(w->plr.stat_timers[i].desc)
		{
			switch(w->plr.stat_timers[i].type)
			{
				case TIMER_STAT:
					apply_stat_mod_desc(NULL, &w->plr.stat_timers[i].desc->stat.effect, 0);
					break;
				case TIMER_FUNC:
					if(w->plr.stat_timers[i].desc->func.apply)
						w->plr.stat_timers[i].desc->func.apply();
					break;
			}
		}
}
//}}}
//{{{
int remove_expired_timers(void)
{
	int ret=0, i;
	for(i=0; i<NUM_STATMOD_TIMERS; i++)
		if(w->plr.stat_timers[i].expiry < w->time && w->plr.stat_timers[i].desc)
		{
			switch(w->plr.stat_timers[i].type)
			{
				case TIMER_STAT:
					if(w->plr.stat_timers[i].desc->stat.expire_message)
						message(w->plr.stat_timers[i].desc->stat.expire_message);
					break;
				case TIMER_FUNC:
					if(w->plr.stat_timers[i].desc->func.expire)
						w->plr.stat_timers[i].desc->func.expire();
					break;
			}
			w->plr.stat_timers[i].desc = NULL;
			ret=1;
		}
	return ret;
}
//}}}
//{{{
void add_timer(const timer_desc *desc, uint duration)
{
	int i;
	
	for(i=0; i<NUM_STATMOD_TIMERS; i++)
	{
		if(w->plr.stat_timers[i].desc == desc)
		{
			if(w->plr.stat_timers[i].expiry < duration+w->time)
				w->plr.stat_timers[i].expiry = duration+w->time;
			goto timer_done;
		}
	}
	for(i=0; i<NUM_STATMOD_TIMERS; i++)
	{
		if(w->plr.stat_timers[i].desc == NULL)
		{
			w->plr.stat_timers[i].type = desc->stat.type;
			w->plr.stat_timers[i].desc = desc;
			w->plr.stat_timers[i].expiry = duration+w->time;
			goto timer_done;
		}
	}
timer_done:
	update_player();
}
//}}}

