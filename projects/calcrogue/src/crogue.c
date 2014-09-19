/*
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
 */
// crogue.c
/// @file
/// @brief Main file for initialization, the event loop, and cleanup.
/// \sa sys/ticalc/main.c
/// \sa sys/curses/main.c

/// @mainpage
///
/// @section Setup Setup and Compiling
///
/// @subsection Tools What you need
/// Before you compile CalcRogue, if you are a Windows user, you will need:
/// @li Cygwin (including make, m4, gcc, g++, cpp, lex, and yacc.)
///      http://www.cygwin.com/
/// @li TIGCC (the very latest version - old versions usually don't work)
///      http://tigcc.ticalc.org/
/// @li The TIGCC Tools Suite
/// @li Doxygen (optional, but can be used to make this documentation)
///      http://www.doxygen.org/
///
/// Linux users, obviously, don't need Cygwin, but they do need working
/// versions of the tools mentioned.
///
/// @subsection Config Configuring
/// Take a look at Makefile.config and make sure all of the 'tool paths'
/// listed are correct. Then look at Makefile.comp, and select the correct
/// compiler (MSVC, GCC, or MinGW).
///
/// @subsection Build Building
/// Now, just type 'make', and (hopefully) everything will build correctly.
///
/// @subsection Editing Editing
/// You can browse CalcRogue's source with any text editor, but it will be
/// much more readable if you have yours set up correctly. Set your tab
/// size to four so that things will line up. Also, if your editor supports
/// it, enable folding; you should have it set so that {{{ and }}} are fold
/// markers and the first non-comment line inside a fold is shown.
///
/// @section GPL Licensing
/// 
/// CalcRogue is copyright (C) 2003 Jim Babcock
/// 
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
/// 
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// The full text of the GNU General Public License may be found in
/// the file COPYING. You should have received this file along with
/// this program; if not, write to the Free Software Foundation,
/// Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include "crogue.h"
#include "substdio.h"
#include "export.h"

#ifdef REGISTERVAR
world *w_non_reg; // A place to back up 'w', because it won't survive in a
                  // register variable if an exception is thrown
#else
world *w;
#endif

static void cb_main_menu_choice(int n, char *buf);
static void main_menu(void);

//{{{
void initialize(void)
{
	int loadedfromfile = 0;
	
	sys_init();
	
	// Claim memory for main data struct
#ifdef REGISTERVAR
	w_non_reg = NULL; // In case call fails, so cleanup will know
#endif
	w = (world*)debug_calloc(1, sizeof(world));
#ifdef REGISTERVAR
	w_non_reg = w;
#endif
	
	lsrand(get_entropy());
	load_options();
	
	// Load saved game if possible
	loadedfromfile = restore_game();
	
#ifdef IS_CALCULATOR
	if(get_option(OPTION_GRAYSCALE) == OPTION_GRAY_ON)
		GrayOnThrow();
#endif
#ifdef REALCOMPUTER
	init_colors();
#endif
	
	// If not loaded from saved game, initialize things
	if(!loadedfromfile) {
		sys_init_data_file();
		display_title_screen();
		
		init_shuffle_tab();
		
		w->messagevis=0;
		w->time=0;
#ifdef ALLOWDEBUG
		w->debug_mode = 0;
#endif
		
		init_spells();
		init_discoveries();
	}
	else
	{
		UI_TF_Init( COORD(MAPSIZE_X,MAPSIZE_Y) );
	}
#ifdef PALMOS
	init_colors();
#endif
	
	init_layout();
	
	if(!loadedfromfile)
		main_menu();
	
	UI_MF_clear();
	UI_TF_PlaceCamera(w->plr.x, w->plr.y);
	calc_light();
	full_redraw();
	if(!loadedfromfile)
		begin_game();
}
//}}}
//{{{
void resize_map_store(int size_x, int size_y)
{
	tile *alloc_pos;
	int ii;
	
	if(w->t && w->mapsize_x == size_x && w->mapsize_y == size_y)
		return;
	
	if(w->t) debug_free(w->t);
	w->mapsize_x = size_x;
	w->mapsize_y = size_y;
	w->t = debug_malloc( size_y*sizeof(tile*)
			+ size_x*size_y*sizeof(tile) );
	alloc_pos = (tile*)((tile**)w->t + size_y);
	for(ii=0; ii<size_y; ii++) {
		w->t[ii] = alloc_pos;
		alloc_pos += size_x;
	}
	
	UI_TF_Init( COORD(size_x, size_y) );
}
//}}}

#ifndef PALMOS
#	define HAVE_TUTORIAL
#endif
//{{{
static const char *main_menu_choices[] = {
	gettext("New game"),
	gettext("Help"),
#ifdef HAVE_TUTORIAL
	gettext("Tutorial"),
#endif
	gettext("High scores"),
	gettext("Options"),
	gettext("Quit")
	};
#ifdef HAVE_TUTORIAL
#define MENU_CHOICES 6
#define MENU_NEWGAME  0
#define MENU_HELP     1
#define MENU_TUTORIAL 2
#define MENU_SCORES   3
#define MENU_OPTIONS  4
#define MENU_QUIT     5
#else
#define MENU_CHOICES 5
#define MENU_NEWGAME  0
#define MENU_HELP     1
#define MENU_SCORES   2
#define MENU_OPTIONS  3
#define MENU_QUIT     4
#endif
//}}}
//{{{
static void cb_main_menu_choice(int n, char *buf)
{
	strcpy(buf, main_menu_choices[n]);
}
//}}}
//{{{
static void main_menu(void)
{
	int choice;
	
restart:
	// Generic title screen method (overlay a menu on a background image, as on calc)
	display_title_screen();
	
	do {
		choice = UI_Menu_Pick( main_menu_rect, MENU_CHOICES, &cb_main_menu_choice);
	} while(choice == -1);
	
	switch(choice)
	{
		case MENU_NEWGAME: // New game
			init_player(0);
			w->current_map_link = w->desc.mainmap;
			w->current_map = (mapdesc*)deref_file_ptr(w->current_map_link);
			w->maxlevel = w->level = w->current_map->mapindex;
			gen_map(0, COORD(0,0));
			break;
		case MENU_HELP: // Help
			help();
			goto restart;
#ifdef HAVE_TUTORIAL
		case MENU_TUTORIAL: // Tutorial
			init_player(1);
			w->current_map_link = w->desc.tutorialmap;
			w->current_map = (mapdesc*)deref_file_ptr(w->current_map_link);
			w->maxlevel = w->level = w->current_map->mapindex;
			gen_map(0, COORD(0,0));
			break;
#endif
		case MENU_SCORES: // High scores
			display_scores(-1);
			goto restart;
		case MENU_OPTIONS: // Options
			options_menu();
			goto restart;
		case MENU_QUIT: // Quit
			quit();
			break;
	}
}
//}}}

//{{{
void eventloop(void)
{
#ifdef PALMOS
	game_running = 1; // Need to know this so AppStopEvent handler knows
	                  // whether to save the game
#endif
	
	while(1)
	{
		plr_move();
		draw();
	}
}
//}}}
//{{{
void timepass(ushort amt)
{
	uint ii;
	
	plr_regenerate(amt);
	
	// Drain torch fuel
	if(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_TIMER)
	{
		for(ii=0; ii<PLRINVSIZE; ii++)
		{
			if(ITEMDESC(INVENTORY(ii)).flags & ITEMFLAG_FUELED)
			{
				if(get_item_property(&INVENTORY(ii), PROPERTY_BURN))
				{
					call_usefunc(get_item_property(&INVENTORY(ii),
						PROPERTY_BURN)->function, ii );
				}
			}
		}
	}
	
	if(remove_expired_timers())
		update_player();
	
	if(w->plr.extrinsic[STAT_DIGESTION] > 0)
	{
		w->plr.satiation -= w->plr.extrinsic[STAT_DIGESTION] * amt;
		if(w->plr.satiation <= -600) {
			message(gettext("You are starving!"));
			plr_takedamage(w->plr.satiation/-600, gettext("Starved to death."));
				// FIXME: PV shouldn't affect this
				// FIXME: should scale with number of turns spend
		}
	}
	
	// Run monster AI
	for(ii=0; ii<MONSTERS_MAX; ii++)
	{
		if(!isNull(w->m[ii].type))
		{
			w->m[ii].energy += MDESC(ii)->speed * amt;
			
			while(w->m[ii].energy >= w->plr.extrinsic[STAT_SPEED] &&
			      !isNull(w->m[ii].type))
			{
				monstmove(ii);
				w->m[ii].energy -= w->plr.extrinsic[STAT_SPEED];
			}
		}
	}
	
	// Add monsters randomly with time
	// (1 in 70 chance per turn)
	for(ii=0; ii<amt; ii++)
		if( !(MAPDESC_CURRENT.flags & MAPFLAG_NORANDMONST) && lrand()%70 == 0 )
		{
#ifdef DEBUG_BALANCE
			if(w->debug_mode)
				message(gettext("Added random monster."));
#endif
			addrandommonst();
		}
	
	// Clear player force-facing
	w->plr.facing_forced = -1;
	if(w->plr.velocity > 0) w->plr.velocity --;
	
	w->time+=amt;
}
//}}}

//{{{
void die(const char *cause_of_death)
{
#ifdef PALMOS
	game_running = 0; // Prevent switching away at this point
#endif
	message(gettext("You die..."));
	draw();
	read_char();
	
	end_game(cause_of_death);
}
//}}}
//{{{
void end_game(const char *cause_of_death)
{
#ifdef PALMOS
	game_running = 0;
	game_repeat = 1;
#endif
	
	award_item_points();
	
#ifdef ALLOWDEBUG
	if(w->debug_mode)
	{
		message( gettext(
			"Since you used debug features, your game will not be scored."));
		read_char();
	} else
#endif
	{
		award_high_score(w->plr.score, cause_of_death);
		display_scores(w->plr.score);
	}
	
	cleanup_save();
	quit();
}
//}}}

//{{{
void quit(void)
{
	cleanup();
	exit(0);
}
//}}}
//{{{
void cleanup(void)
{
	sys_cleanup();
	
	if(w) {
		if(w->t)                  debug_free(w->t);
		if(w->plr.spellknowledge) debug_free(w->plr.spellknowledge);
		if(w->shuffletranslation) debug_free(w->shuffletranslation);
		if(w->itemids)            debug_free(w->itemids);
		cleanup_items();
		debug_free(w);
	}
	UI_TF_Deinit();
	
	exit(0);
}
//}}}


