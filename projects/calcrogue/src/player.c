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
// player.c
/// @file
/// @brief Controls, and functions affecting the state of the player avatar

#include "crogue.h"
#include "export.h"

static sint mod_from_attribute(uint attr, sint mindistance, sint divisor);

static void plr_walk(sshort x, sshort y);
static void plr_run(sshort x, sshort y);
#ifdef KEY_RUN
static void plr_run_prompt(void);
#endif
#ifdef KEY_SCROLL_PROMPT
static void scroll_prompt(void);
#endif
static void plr_attack( int monstid );
static void manip_door(int open);

//{{{
/// Make an attack-roll
int plr_attack_monster_roll(int monstid, int modifier)
{
	return calculate_hit(w->plr.level*2 + modifier, MDESC(monstid)->dv);
}
//}}}

//{{{
/// @brief Callback for class name
///
/// Copies the name of class @a n into @a buf.
/// @sa UI_Menu_Pick
static void playerclassname(int n, char *buf)
{
	strcpy( buf, (char*)deref_file_ptr(w->playerclasses[n].name) );
}
//}}}
//{{{
/// @brief Determine how much something should be modified based on a stat
///
/// A stat is modified by @a attr by an amount depending on how good or
/// bad it is. If @a attr is average (10), then there is no change. @a
/// mindistance, then, is how far from average a stat must be before
/// there is any noticeable effect, and @a divisor is how many points
/// beyond that it takes to make a difference of 1 in the result.
/// @a attr must be one of the @ref playerstats "Player Stats".
/// @sa playerstats player::extrinsic update_player
static sint mod_from_attribute(uint attr, sint mindistance, sint divisor)
{
	sint v = w->plr.extrinsic[attr];
	
	v -= 10;
	
	if(v>0)
	{
		v -= mindistance;
		if(v<0)
			v=0;
	}
	if(v<0)
	{
		v += mindistance;
		if(v>0)
			v=0;
	}
	
	v /= divisor;
	
	return v;
}
//}}}
//{{{
/// Initialize the player's class and stats
void init_player(uint is_tutorial)
{
	const struct classdesc *plr_class;
	int i;
	
	if(is_tutorial)
	{
		w->plr.class = w->desc.tutorialclass;
	} else {
		display_title_screen();
		// Pick player class
		do {
#ifdef USE_CURSES
			w->plr.class = UI_Menu_Pick( class_menu_rect, w->desc.numplayerclasses, &playerclassname );
#else
			w->plr.class = UI_Menu_Pick( class_menu_rect, w->desc.numplayerclasses, &playerclassname );
#endif
		} while(w->plr.class==-1);
	}
	plr_class = &w->playerclasses[w->plr.class];
	
	init_inventory();
	
	// Set up statistics and timers
	for(i=0; i<NUM_STATS; i++)
		w->plr.intrinsic[i] = 0;
	
	distribute_attributes();
	init_timers();
	
	w->plr.intrinsic[STAT_LIGHTRADIUS] = 1;
	w->plr.extrinsic[STAT_LIGHTRADIUS] = -1;
	w->plr.intrinsic[STAT_SPEED] = 10;
	w->plr.intrinsic[STAT_DIGESTION] = 12;
	
	w->plr.hps = w->plr.hps_max = w->plr.intrinsic[STAT_TOUGHNESS] +
	                              w->plr.intrinsic[STAT_WILLPOWER]/3;
	w->plr.hps_max_mod = 0;
	w->plr.pps_max_mod = 0;
	w->plr.pps = w->plr.pps_max = w->plr.intrinsic[STAT_MANA];
	w->plr.partialhps = w->plr.partialpps = 0;
	w->plr.level = 1;
	w->plr.xp = 0;
	w->plr.satiation = 12000l;
	w->plr.debt = 0;
	w->plr.facing = w->plr.facing_forced = -1;
	w->plr.velocity = 0;
	
	w->plr.score = 0;
	
	update_player();
	init_skills();
	update_player();
}
//}}}
//{{{
void apply_stat_mod_desc(sint *target, const stat_mod_desc *s, schar plus)
{
	if(!target)
		target = &w->plr.extrinsic[s->stat];
	
	switch(s->type)
	{
		case STATMOD_ADD:
			*target += s->amount;
			break;
		case STATMOD_ADDPLUS:
			*target += s->amount + plus;
			break;
		case STATMOD_OR:
			*target |= s->amount;
			break;
		case STATMOD_SET:
			*target = s->amount;
			break;
	}

}
//}}}
//{{{
/// Recalculate player's extrinsic stats
void update_player(void)
{
	int i;
	int old_lightradius = w->plr.extrinsic[STAT_LIGHTRADIUS];
	item* item;
	
	for(i=0; i<NUM_STATS; i++)
		w->plr.extrinsic[i] = w->plr.intrinsic[i];
	
	for(i=0; i<PLRINVSIZE; i++)
	{
		item = &INVENTORY(i);
		if(!item->type) continue;
		if(!(item->flags & ITEMFLAG_EQUIPPED)) continue;
		
		apply_all_item_properties(item, PROPERTY_WORN_STAT_EFFECT);
	}
	apply_timers();
	for(i=0; i<PLRINVSIZE; i++)
	{
		item = &INVENTORY(i);
		if(!item->type) continue;
		apply_all_item_properties(item, PROPERTY_CARRIED_EFFECT);
	}
	
	w->plr.extrinsic[STAT_DV]         += mod_from_attribute(STAT_DEXTERITY, 0, 2);
	w->plr.extrinsic[STAT_PV]         += mod_from_attribute(STAT_TOUGHNESS, 3, 6);
	w->plr.extrinsic[STAT_SPEED]      += mod_from_attribute(STAT_DEXTERITY, 3, 3);
	w->plr.extrinsic[STAT_TOHIT]      += w->plr.extrinsic[STAT_STRENGTH]/4;
	w->plr.extrinsic[STAT_TOHIT]      += weapon_skill_attack();
	w->plr.extrinsic[STAT_MISSILE_TOHIT] += mod_from_attribute(STAT_DEXTERITY, 2, 2);
	w->plr.extrinsic[STAT_DAMAGE_MAX] += w->plr.extrinsic[STAT_STRENGTH]/4
	                                  + mod_from_attribute(STAT_STRENGTH, 5, 3);
	w->plr.extrinsic[STAT_DAMAGE_MAX] += weapon_skill_damage();
	w->plr.extrinsic[STAT_MISSILEDAM_MAX] += w->plr.extrinsic[STAT_DEXTERITY]/4;
	w->plr.extrinsic[STAT_DAMAGE_MIN] += weapon_skill_damage()
	                                  + mod_from_attribute(STAT_STRENGTH, 5, 4);
	
	if(w->plr.extrinsic[STAT_LIGHTRADIUS] > 0) {
	                               // Ordinarily this would never be <= zero,
	                               // but grues can make things *really* dark
		w->plr.extrinsic[STAT_LIGHTRADIUS]*=w->plr.extrinsic[STAT_LIGHTRADIUS];
		w->plr.extrinsic[STAT_LIGHTRADIUS]++;
	} else
		w->plr.extrinsic[STAT_LIGHTRADIUS] = 0;
	
	w->plr.hps_max = w->plr.extrinsic[STAT_TOUGHNESS]
	                 + w->plr.extrinsic[STAT_WILLPOWER] / 3
	                 + max(1, (4 + mod_from_attribute(STAT_TOUGHNESS, 0, 2))) * (w->plr.level - 1)
					 + max(0, (2 + mod_from_attribute(STAT_WILLPOWER, 0, 3))) * (w->plr.level - 1)
					 + w->plr.hps_max_mod;
	w->plr.pps_max = w->plr.extrinsic[STAT_MANA]
	                 + max(0, (3 + mod_from_attribute(STAT_MANA, 0, 2))) * (w->plr.level - 1)
					 + max(0, mod_from_attribute(STAT_WILLPOWER, 0, 3)) * (w->plr.level - 1)
					 + w->plr.pps_max_mod;
	if(w->plr.pps_max < 0) w->plr.pps_max = 0;
	if(w->plr.hps_max < 1) w->plr.hps_max = 1;
	
	if(w->plr.extrinsic[STAT_LIGHTRADIUS] != old_lightradius
	   && old_lightradius != -1)
		calc_light();
}
//}}}
//{{{
/// A new map has been entered
void enter_map(void)
{
	w->interrupt = 1;
	UI_TF_PlaceCamera(w->plr.x, w->plr.y);
	call_genericfunc(MAPDESC_CURRENT.enterfunction);
	calc_light();
	full_redraw();
}
//}}}
//{{{
/// Follow a staircase which the player is standing on
void follow_connection(void)
{
	const spec_descriptor *connection;
	tile *t = &w->t[w->plr.y][w->plr.x];
	filelink target_level_link;
	
	if( TILEDESC(*t).is_connection==0 ) {
		message(gettext("You can't go up or down from here."));
		return;
	}
	
	connection = &MAPSPEC(t->special);
	target_level_link = connection->ext.lvl.destmap;
	
	goto_level(target_level_link, &connection->ext.lvl);
	
	timepass(1);
}
//}}}
//{{{
void goto_level(filelink target_level_link,
                const spec_descriptor_ext_level *connection)
{
	int source_level;
	const mapdesc *target_level;
	
	target_level = (const mapdesc*)deref_file_ptr(target_level_link);
	
	// If these are the up stairs from the first level (where you came in), then
	// climbing them is the same as quitting. So ask first.
	if( target_level->flags & MAPFLAG_OUTSIDE )
	{
		if(prompt(gettext("Leave the dungeon?")))
			end_game(gettext("Left the dungeon"));
		else
			return;
	}
	
	// Make a backup saved game before leaving
	if(get_option(OPTION_CHECKPOINT) == OPTION_YES)
		save_checkpoint();
	
	gather_wandering_monsters();
	w->t[w->plr.y][w->plr.x].flags &= ~TFLAG_OCCUPIED;
	
	save_level();
	
	source_level = w->level;
	w->current_map_link = target_level_link;
	w->current_map = target_level;
	w->level = target_level->mapindex;
	w->maxlevel = max(w->maxlevel, w->level);
	if( !load_level(source_level, connection->destcoord) ) {
		gen_map(source_level, connection->destcoord );
	}
	
	enter_map();
	
	if( !(target_level->flags & MAPFLAG_NOMONST) ) {
		place_wandering_monsters();
	}
}
//}}}
//{{{
/// Read and act on a keypress
void plr_move(void)
{
	int move;
	
	if(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_PARALYZED)
	{
		timepass(1);
		return;
	}
	
move_again:
	move = read_char();
	
	if(move==KEY_ACT_MENU)
		move = actions_menu();
	
	switch(move) {
		
		case KEY_NOP:
			break;
		
		/////////////////////////// MOVEMENT //////////////////////////////////
		
		case KEY_SOUTHWEST:     plr_walk(-1,  1);  break;
		case KEY_SOUTH:         plr_walk( 0,  1);  break;
		case KEY_SOUTHEAST:     plr_walk( 1,  1);  break;
		case KEY_WEST:          plr_walk(-1,  0);  break;
		case KEY_EAST:          plr_walk( 1,  0);  break;
		case KEY_NORTHWEST:     plr_walk(-1, -1);  break;
		case KEY_NORTH:         plr_walk( 0, -1);  break;
		case KEY_NORTHEAST:     plr_walk( 1, -1);  break;
		
		case KEY_RUN_SOUTHWEST: plr_run( -1,  1);  break;
		case KEY_RUN_SOUTH:     plr_run(  0,  1);  break;
		case KEY_RUN_SOUTHEAST: plr_run(  1,  1);  break;
		case KEY_RUN_WEST:      plr_run( -1,  0);  break;
		case KEY_RUN_EAST:      plr_run(  1,  0);  break;
		case KEY_RUN_NORTHWEST: plr_run( -1, -1);  break;
		case KEY_RUN_NORTH:     plr_run(  0, -1);  break;
		case KEY_RUN_NORTHEAST: plr_run(  1, -1);  break;
#ifdef KEY_RUN
		case KEY_RUN:           plr_run_prompt();  break;
#endif

#ifdef KEY_FIRING
		case KEY_FIRE_SOUTHWEST: fire_dir( -1,  1);  break;
		case KEY_FIRE_SOUTH:     fire_dir(  0,  1);  break;
		case KEY_FIRE_SOUTHEAST: fire_dir(  1,  1);  break;
		case KEY_FIRE_WEST:      fire_dir( -1,  0);  break;
		case KEY_FIRE_EAST:      fire_dir(  1,  0);  break;
		case KEY_FIRE_NORTHWEST: fire_dir( -1, -1);  break;
		case KEY_FIRE_NORTH:     fire_dir(  0, -1);  break;
		case KEY_FIRE_NORTHEAST: fire_dir(  1, -1);  break;
#endif
	 	
		case KEY_WAIT:
			timepass(1);
			break;
		
		case KEY_STAIR:	// Use stair
			follow_connection();
			break;
		
		/////////////////////////// INVENTORY /////////////////////////////////

		case KEY_INVENTORY:	// Show inventory
			if(!count_inventory())
				message(gettext("You are empty-handed."));
			else
				pick_item(filter_none);
			break;
		
		case KEY_WEAR:	// Wear an item
			wear_item();
			break;
		
		case KEY_WEAR_MULTIPLE:
			wear_multiple();
			break;
			
		case KEY_TAKEOFF:	// Take off
			takeoff_item();
			break;
			
		case KEY_TAKEOFF_MULTIPLE:
			takeoff_multiple();
			break;
			
		case KEY_DROP:	// Drop an item
			drop_item();
			break;
		
		case KEY_DROP_MULTIPLE:
			drop_multiple();
			break;
				
		case KEY_PICKUP:	// Pick up an item
			if( count_items(w->plr.x, w->plr.y) == 0 ) {
				message("There is nothing here to pick up.");
			} else {
				// Prompt if more than 1 item
				pickup(count_items(w->plr.x, w->plr.y) > 1);
			}
			break;
		
		case KEY_CAST:
			cast_spell();
			break;
			
		case KEY_USE:	// Use an item
			use_item();
			break;
		
		case KEY_THROW:
			throw_item_from_menu();
			break;
			
		case KEY_FIRE:
			throw_readied_item();
			break;
		
#ifdef SUPPORT_HOTKEY_ITEM
		case KEY_HOTKEY_ITEM:
			hotkey_item();
			break;
#endif
		
		///////////////////////////// MISC ////////////////////////////////////
		
		case KEY_MESSAGE_HISTORY:
			UI_MF_history();
			break;
			
		case KEY_DISCOVERIES:
			list_discoveries();
			break;
		
		case KEY_SEARCH:	// Search
			plr_search();
			break;
		
		case KEY_ESC:	// ESC - quit
			if( prompt(gettext("Really quit?")) )
			{
				if(prompt(gettext("Save this game?"))) {
					save_game();
					quit();
				} else {
					end_game(gettext("Quit"));
				}
			}
			break;
		
		case KEY_SAVEGAME:	// On
			#ifdef IS_CALCULATOR // Fast boss-button response
			GrayOff();
			LCD_restore(saved_screen);
			#endif
			save_game();
			quit();
			break;
		
		case KEY_HELPMENU:
			help();
			full_redraw();
			break;
		case KEY_STATS:
			char_info();
			break;
		case KEY_ABOUT:
			message(gettext("CalcRogue by Jim Babcock, 2002. This program is postcardware. If you enjoy it, send the author a postcard at 16 Sweeney Ridge Rd, Bedford, MA, USA, ZIP code 01730"));
			break;
		
		case KEY_CLOSEDOOR:	// Close door
			manip_door(0);
			break;
		case KEY_OPENDOOR:	// Open door
			manip_door(1);
			break;
		
#		ifdef KEY_SCROLLING
			case KEY_SCROLL_RIGHT:
				UI_TF_Scroll(KEY_SCROLL_SIZE, 0);
				break;
			case KEY_SCROLL_DOWN:
				UI_TF_Scroll(0, KEY_SCROLL_SIZE);
				break;
			case KEY_SCROLL_UP:
				UI_TF_Scroll(0, -KEY_SCROLL_SIZE);
				break;
			case KEY_SCROLL_LEFT:
				UI_TF_Scroll(-KEY_SCROLL_SIZE, 0);
				break;
#			ifdef KEY_SCROLLING_DIAGONAL
				case KEY_SCROLL_UPRIGHT:
					UI_TF_Scroll(KEY_SCROLL_SIZE, -KEY_SCROLL_SIZE);
					break;
				case KEY_SCROLL_DOWNRIGHT:
					UI_TF_Scroll(KEY_SCROLL_SIZE, KEY_SCROLL_SIZE);
					break;
				case KEY_SCROLL_UPLEFT:
					UI_TF_Scroll(-KEY_SCROLL_SIZE, -KEY_SCROLL_SIZE);
					break;
				case KEY_SCROLL_DOWNLEFT:
					UI_TF_Scroll(-KEY_SCROLL_SIZE, KEY_SCROLL_SIZE);
					break;
#			endif
#			ifdef KEY_SCROLL_PROMPT
				case KEY_SCROLL_PROMPT:
					scroll_prompt();
					break;
#			endif
#		endif
		
		case KEY_REDRAW:
			Graph_ClrScr();
			draw_all_tiles();
			full_redraw();
			break;
		
		case KEY_OPTIONS_MENU:
			options_menu();
			init_layout();
			UI_TF_MoveCamera(w->plr.x, w->plr.y);
			Graph_ClrScr();
			full_redraw();
			break;
			
		///////////////////////////// DEBUG ///////////////////////////////////
		#ifdef ALLOWDEBUG
			case KEY_DEBUG:
				debug();
				break;
		#endif
	
		default:   // Message the keycode of any unrecognized key pressed
			#ifndef USE_CURSES // ... but not in curses, since some versions
			                   // send keystrokes for meta-keys and such
				#ifdef ALLOWDEBUG
					if(w->debug_mode)
						message(gettext("Unrecognized key code: %i"), move);
//					else
//						message(gettext("Invalid command."));
//				#else
//					message(gettext("Invalid command."));
				#endif
			#endif
			
				// That wasn't an actual move, so it's not time to return yet
			goto move_again;
	}
	return;
}
//}}}
//{{{
/// Teleport to a random spot
void place_player_randomly(void)
{
	int x, y;
	
	do
	{
		x = RANGE(MAPSIZE_X-1, 0);
		y = RANGE(MAPSIZE_Y-1, 0);
	} while( !(w->tiledescs[w->t[y][x].type].passable) ||
	         w->t[y][x].flags & TFLAG_OCCUPIED );
	
	plr_moveto(x, y, -1);
}
//}}}
//{{{
/// Try to walk in the given direction
static void plr_walk(sshort x, sshort y)
{
	int d;
	int velmod = 0;
	if( w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_ENTANGLED ) {
		message(gettext("You are stuck in the webs!"));
		timepass(1);
		return;
	}
	if( w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_CONFUSED ) {
		message(gettext("You are confused."));
		d = RANGE(7,0);
		x = dir[d][0];
		y = dir[d][1];
	}
	if( w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_SLIPPERY )
	{
		if( 2*delta_facing(player_facing(), facing(x, y))
			 *delta_facing(player_facing(), facing(x, y))
			 *(w->plr.velocity+1)
			+ RANGE(8,0)
			> 3*w->plr.extrinsic[STAT_DEXTERITY] )
		{
			message(gettext("You slip and fall!"));
			w->plr.velocity = 0;
			w->plr.facing = facing(x, y);
			timepass(1);
			return;
		}
	}
	if(w->plr.facing == facing(x,y))
		velmod = 1;
	else if(delta_facing(player_facing(), facing(x,y)) > 2)
		w->plr.velocity = 0;
	w->plr.facing = facing(x, y);
	
	x += w->plr.x;
	y += w->plr.y;
	
	if( w->t[y][x].flags & TFLAG_OCCUPIED && (w->plr.x != x || w->plr.y != y) )
	{
		int target = monstbytile(x, y);
		if(monst_is_pet(target))
		{
			if(TILEDESC(w->t[y][x]).passable)
			{
				plr_moveto(x, y, target);
				timepass(1);
			}
			return;
		}
		else if(monst_is_peaceful(target)
		        && !(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_CONFUSED)) {
			if(!prompt( retprintf(gettext("Really attack the %s?"), monstname(target)) ))
				return;
		}
		plr_attack( monstbytile(x, y) );
		timepass(1);
		return;
	}
	
	if( TILEDESC(w->t[y][x]).passable )
		plr_moveto(x, y, -1);
	else
	{
		w->interrupt = 1;
		
/*		if(w->t[y][x].type==TILE_CDOOR) {
			w->t[y][x].type = TILE_ODOOR;
			draw_tile(x, y);
			calc_light();
		} else */
		if( !isNull(TILEDESC(w->t[y][x]).step_function) ) {
			plr_step(x, y);
		} else {
			if(!(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_CONFUSED))
				return;
		}
	}
	
	w->plr.velocity += velmod + 1;
	timepass(1);
}
//}}}
//{{{
void plr_step(sshort x, sshort y)
{
	if( !isNull(TILEDESC(w->t[y][x]).step_function) )
		call_stepfunc( TILEDESC(w->t[y][x]).step_function, x, y);
}
//}}}
//{{{
/// @brief Try to run in the given direction
///
/// This function causes the player to keep moving in the given direction
/// until w->interrupt is set, or a wall is reached.
///
/// @sa world::interrupt
static void plr_run(sshort x, sshort y)
{
	w->interrupt = 0;
	
	do
	{
		plr_walk(x, y);
#ifndef PALMOS
		if(key_pending())
			break;
#endif
	}
	while(!w->interrupt);
}
//}}}
#ifdef KEY_RUN
//{{{
static void plr_run_prompt(void)
{
	direction dir;
	prompt_dir("Run which way?", &dir);
	plr_run(dir.x, dir.y);
}
//}}}
#endif
#ifdef KEY_SCROLL_PROMPT
//{{{
static void scroll_prompt(void)
{
	direction dir;
	prompt_dir("Scroll which way?", &dir);
	UI_TF_Scroll(dir.x*KEY_SCROLL_SIZE, dir.y*KEY_SCROLL_SIZE);
}
//}}}
#endif
//{{{
/// Attack the monster with given index
static void plr_attack( int monstid )
{
	sint damage;
	
	if(monst_attack_type(MTRIGGER_PREEMPT, monstid, 1))
		return;
	
	monst_anger(monstid);
	damage = nrandom(w->plr.extrinsic[STAT_DAMAGE_MAX], w->plr.extrinsic[STAT_DAMAGE_MIN]);
#ifdef DEBUG_BALANCE
	if(w->debug_mode)
		message("(%i,%i)v[%i,%i]{%i/%i}",
			w->plr.extrinsic[STAT_TOHIT] + w->plr.level*2, damage,
			MDESC(monstid)->dv, MDESC(monstid)->pv,
			w->m[monstid].hps, MDESC(monstid)->hps_max);
#endif
	if( plr_attack_monster_roll(monstid, w->plr.extrinsic[STAT_TOHIT]) )
	{
		message(gettext("You hit the %s."), monstname(monstid));
		
		monst_takedamage(monstid, damage, 1);
	} else {
		message(gettext("You miss the %s."), monstname(monstid));
	}
	award_mark();
}
//}}}
//{{{
/// @brief Messages shown when a stat is drained, boosted, or restored
static const char* statmsg[3][5] = {
	{
		gettext("You feel weaker."),
		gettext("You feel clumsy."),
		gettext("You feel sickly."),
		gettext("You feel tired."),
		gettext("You feel naive."),
	},
	{
		gettext("You feel stronger."),
		gettext("Your coordination improves."),
		gettext("You feel tougher."),
		gettext("You feel powerful."),
		gettext("You feel skeptical."),
	},
	{
		gettext("You feel your strength returning."),
		gettext("You feel less clumsy."),
		gettext("You feel restored to health."),
		gettext("You feel restored."),
		gettext("You feel less naive."),
	}
  };
//}}}
//{{{
/// Drain or boost stat @a which by given @a amt
void plr_mod_stat(sint which, sint amt)
{
	message(statmsg[amt>0][which]);
	
	if( amt <= 0 ) {
		if(w->plr.intrinsic[which] > -amt)
			w->plr.intrinsic[which] += amt;
		else
			w->plr.intrinsic[which] = 1;
	} else {
		w->plr.intrinsic[which] += amt;
		if(w->plr.maximum[which] < w->plr.intrinsic[which])
			w->plr.maximum[which] = w->plr.intrinsic[which];
	}
	
	update_player();
}
//}}}
//{{{
/// Restore stat @a which by @a amt if it has been drained
short plr_restore_stat(short which, short amt)
{
	if(w->plr.intrinsic[which] >= w->plr.maximum[which])
		return 0;
	message(statmsg[2][which]);
	w->plr.intrinsic[which] += amt;
	if(w->plr.intrinsic[which] > w->plr.maximum[which])
		w->plr.intrinsic[which] = w->plr.maximum[which];
	update_player();
	return 1;
}
//}}}
//{{{
/// @brief Move to the given location
///
/// The player is moved to the given location, possibly teleporting.
/// This function handles entering/leaving shops, auto-pickup, and traps.
void plr_moveto(ushort x, ushort y, sshort displace_monst)
{
	int was_in_shop=0;
	int old_x = w->plr.x, old_y = w->plr.y;
	
	if(displace_monst >= 0)
	{
		w->m[displace_monst].x = old_x;
		w->m[displace_monst].y = old_y;
	} else {
		w->t[old_y][old_x].flags &= ~TFLAG_OCCUPIED;
	}
	
	if(w->t[old_y][old_x].type == TILE_SHOPFLOOR)
		was_in_shop = 1;
	
	w->plr.x = x;
	w->plr.y = y;
	draw_tile(old_x, old_y);	// Redraw the tile the player stepped off of
	w->t[w->plr.y][w->plr.x].flags |= TFLAG_OCCUPIED | TFLAG_LIT | TFLAG_EXPLORED;
	UI_TF_MoveCamera(w->plr.x, w->plr.y);
	draw_tile(w->plr.x, w->plr.y);	// Redraw the tile the player stepped onto
	calc_light();
	
	if(!was_in_shop && w->t[y][x].type == TILE_SHOPFLOOR)
	{
		message(gettext("You enter %s."), deref_file_ptr(w->shopdescs[w->t[y][x].special].name));
	}

	if(count_items(x, y) > 0)
		item_step();
	
	plr_step(x, y);
}
//}}}
//{{{
/// Open or close a door
static void manip_door(int open)
{
	int x=0, y=0;
	int xi, yi, n=0;
	direction d;
	int success=0;
	
	for(yi=w->plr.y-1; yi<=w->plr.y+1; yi++)
	for(xi=w->plr.x-1; xi<=w->plr.x+1; xi++)
	{
		if(w->t[yi][xi].type == TILE_ODOOR || w->t[yi][xi].type == TILE_CDOOR)
		{
			n++;
			x = xi;
			y = yi;
		}
	}
	
	if(n != 1)
	{
		prompt_dir(gettext("In what direction?"), &d);
		x = w->plr.x + d.x;
		y = w->plr.y + d.y;
	}
	
	if( open ) {
		if( w->t[y][x].type == TILE_CDOOR ) {
			w->t[y][x].type = TILE_ODOOR;
			success=1;
		}
	} else {
		if( w->t[y][x].type == TILE_ODOOR ) {
			if(monstbytile(x, y) != -1 || count_items(x, y) > 0)
				message(gettext("The door is blocked."));
			else
				w->t[y][x].type = TILE_CDOOR;
			success=1;
		}
	}
	calc_light();
	
	if( !success )
		message(gettext("You see no door there."));
	else
	{
		draw_tile(x, y);
		timepass(1);
	}
}
//}}}
//{{{
/// Search for traps and secret doors
void plr_search(void)
{
	int x, y;
	
	message("Searching.");
	
	timepass(1);
	
	for(y=w->plr.y-1; y<=w->plr.y+1; y++)
		for(x=w->plr.x-1; x<=w->plr.x+1; x++)
		{
			if(w->t[y][x].type == TILE_SDOOR && RANGE(1,0)==0)
				// Found a secret door
			{
				w->t[y][x].type = TILE_CDOOR;
				message(gettext("You found a secret door!"));
				draw_tile(x, y);
				awardXP(5);
			}
			if(w->t[y][x].flags & TFLAG_HIDETRAP && RANGE(1,0)==0)
				// Found a trap
			{
				w->t[y][x].flags &= ~TFLAG_HIDETRAP;
				message(gettext("You found a trap!"));
				draw_tile(x, y);
				awardXP(10);
			}
		}
}
//}}}
//{{{
/// Display player stats
void char_info(void)
{
	char outbuf[2048];
	Graph_ClrScr();
	sprintf(
		outbuf,
		gettext("Level %i %s\n"
		"\n"
		"Strength\t%i\t (%i)\n"
		"Dexterity\t%i\t (%i)\n"
		"Toughness\t%i\t (%i)\n"
		"Mana\t%i\t (%i)\n"
		"Willpower\t%i\t (%i)\n"
		"\n"
		"Speed %i\n"
		"DV %i(%i)\n"
		"PV %i(%i)\n"
		"Damage %i-%i\n"
		"\n"
		"Weapon skills\n"),
		(int)w->plr.level,                (const char*)deref_file_ptr(w->playerclasses[w->plr.class].name),
		(int)w->plr.intrinsic[STAT_STRENGTH],   (int)w->plr.extrinsic[STAT_STRENGTH],
		(int)w->plr.intrinsic[STAT_DEXTERITY],  (int)w->plr.extrinsic[STAT_DEXTERITY],
		(int)w->plr.intrinsic[STAT_TOUGHNESS],  (int)w->plr.extrinsic[STAT_TOUGHNESS],
		(int)w->plr.intrinsic[STAT_MANA],       (int)w->plr.extrinsic[STAT_MANA],
		(int)w->plr.intrinsic[STAT_WILLPOWER],  (int)w->plr.extrinsic[STAT_WILLPOWER],
		(int)w->plr.extrinsic[STAT_SPEED],
		(int)w->plr.extrinsic[STAT_DV],         (int)w->plr.intrinsic[STAT_DV],
		(int)w->plr.extrinsic[STAT_PV],         (int)w->plr.intrinsic[STAT_PV],
		(int)w->plr.extrinsic[STAT_DAMAGE_MIN], (int)w->plr.extrinsic[STAT_DAMAGE_MAX]
	);
	print_skills(outbuf);
	
	setTabStops(player_screen_tabs);
#ifdef IS_CALCULATOR
	SetFont(OPTION_FONT_SMALL);
#endif
	draw_string(outbuf, NULL, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	read_char();
	full_redraw();
}
//}}}

