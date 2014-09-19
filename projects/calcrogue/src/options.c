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
// options.c
/// @file
/// @brief Player-configurable options menu

#include "crogue.h"
#include "export.h"

#ifndef PALMOS
#	define DEFAULT_AUTOPICKUP OPTION_PROMPT
#	define DEFAULT_AUTOFOLLOW OPTION_PROMPT
#else
#	define DEFAULT_AUTOPICKUP OPTION_NO
#	define DEFAULT_AUTOFOLLOW OPTION_NO
#endif

//{{{
static const char *choices_yesnoprompt[] = {
	gettext("Yes"),
	gettext("No"),
	gettext("Prompt"),
	NULL };
//}}}
//{{{
static const char *choices_yesno[] = {
	gettext("Yes"),
	gettext("No"),
	NULL };
//}}}
#ifdef IS_CALCULATOR
//{{{
static const char *choices_smalllarge[] = {
	gettext("Small"),
	gettext("Large"),
	NULL };
//}}}
//{{{
static const char *choices_fontsize[] = {
	gettext("Small"),
	gettext("Medium"),
	gettext("Large"),
	NULL };
//}}}
#endif

//{{{
/// A configuration option
typedef struct option
{
	const char *description;
	ulong key;
	const char **choices;
	uchar default_value;
} option;
//}}}

//{{{
static const option options[] = {
		{ gettext("Auto-pickup"),
		  OPTION_AUTOPICKUP,
		  choices_yesnoprompt,
		  DEFAULT_AUTOPICKUP },
		{ gettext("Auto-follow-stairs"),
		  OPTION_AUTOFOLLOW,
		  choices_yesnoprompt,
		  DEFAULT_AUTOFOLLOW },
#ifdef SUPPORT_COLOR
		{ gettext("Use color"),
		  OPTION_COLOR,
		  choices_yesno,
		  OPTION_YES },
#endif
		{ gettext("Checkpoint saves"),
		  OPTION_CHECKPOINT,
		  choices_yesno,
		  OPTION_YES },
#ifdef PALMOS
		{ gettext("Use dialog for prompts"),
		  OPTION_PROMPT_DLG,
		  choices_yesno,
		  OPTION_YES },
		{ gettext("Invert colors"),
		  OPTION_INVERSE,
		  choices_yesno,
		  OPTION_NO },
#endif
#ifdef IS_CALCULATOR
		{ gettext("Tile size"),
		  OPTION_TILESIZE,
		  choices_smalllarge,
		  OPTION_SIZE_SMALL },
		{ gettext("Font size"),
		  OPTION_FONTSIZE,
		  choices_fontsize,
		  OPTION_FONT_SMALL },
		{ gettext("Archive saves"),
		  OPTION_AUTOARCHIVE,
		  choices_yesno,
		  OPTION_NO },
		{ gettext("Archive scores"),
		  OPTION_ARCHIVESCORES,
		  choices_yesno,
		  OPTION_YES },
		{ gettext("Archive options"),
		  OPTION_ARCHIVEOPTIONS,
		  choices_yesno,
		  OPTION_YES },
		{ gettext("Use grayscale"),
		  OPTION_GRAYSCALE,
		  choices_yesno,
		  OPTION_YES },
#endif
		{ NULL },
	};
//}}}
//{{{
static void cb_options_menu(int n, char *buf)
{
#ifdef OPTIONS_SUBMENUS
	if(n < OPTIONS_SUBMENUS) {
		options_submenu(n, buf);
		return;
	}
	n -= OPTIONS_SUBMENUS;
#endif
	sprintf(buf, "%s\t%s", options[n].description,
		options[n].choices[ get_option(options[n].key) ]);
}
//}}}

typedef struct option_value
{
	ushort key;
	ushort value;
} option_value;
option_value *options_set = NULL;
ulong num_options_set=0;

//{{{
/// Return the currently-set value for the given option. If no value has been
/// set, return the default for that option. If there is no default, return -1.
int get_option(int option)
{
	int ii;
	int num_opts;
	// Find option setting
	for(ii=0; ii<num_options_set; ii++) {
		if(options_set[ii].key == option)
			return options_set[ii].value;
	}
	num_opts = count_options();
	// Find default value
	for(ii=0; ii<num_opts; ii++) {
		if(options[ii].key == option)
			return options[ii].default_value;
	}
	return -1;
}
//}}}
//{{{
void set_option(int option, int value)
{
	int ii;
	
	// Look for pre-existing setting of this option
	for(ii=0; ii<num_options_set; ii++) {
		if(options_set[ii].key == option) {
			options_set[ii].value = value;
			return;
		}
	}
	options_set = (option_value*)realloc(options_set,
		++num_options_set * sizeof(option_value));
	options_set[num_options_set-1].key = option;
	options_set[num_options_set-1].value = value;
}
//}}}
//{{{
int count_options(void)
{
	int ii;
	for(ii=0; ; ii++)
	{
		if(!options[ii].description)
			return ii;
	}
}
//}}}
//{{{
int count_choices(const char **choices)
{
	int ret=0;
	while(*choices != NULL) {
		choices++;
		ret++;
	}
	return ret;
}
//}}}

//{{{
void options_menu(void)
{
	int choice;
	int old_value, new_value;
	int change_made = 0;
	
	setTabStops(options_menu_tabs);
	UI_Menu_Set_Persist(1);
	while(1) {
#ifdef OPTIONS_SUBMENUS
		choice = UI_Menu_Pick(options_rect, count_options()+OPTIONS_SUBMENUS,
		                      &cb_options_menu);
		if(choice>=0 && choice<OPTIONS_SUBMENUS) {
			pick_submenu(choice);
			change_made = 1;
			continue;
		} else {
			choice -= OPTIONS_SUBMENUS;
		}
#else
		choice = UI_Menu_Pick(options_rect, count_options(), &cb_options_menu);
#endif
		
		if(choice<0)
			break;
		old_value = get_option(options[choice].key);
		new_value = (old_value+1) % count_choices(options[choice].choices);
		set_option(options[choice].key, new_value);
		change_made = 1;
		
#ifdef IS_CALCULATOR
		if(options[choice].key == OPTION_GRAYSCALE)
		{
			if(get_option(OPTION_GRAYSCALE) == OPTION_GRAY_ON)
				GrayOnThrow();
			else
				GrayOff();
			if(w->level) // Only if game is started
				full_redraw();
		}
#endif
#ifdef PALMOS
		if(options[choice].key == OPTION_INVERSE)
			init_colors();
#endif
#ifdef REALCOMPUTER
		if(options[choice].key == OPTION_COLOR)
			init_colors();
#endif
	}
	UI_Menu_Set_Persist(0);
	
	if(change_made)
		save_options();
}
//}}}

//{{{
void load_options(void)
{
	FILE *configfile;
	ulong num_settings;
	option_value entry;
	int ii;
	
#ifdef REALCOMPUTER
	char full_filename[256];
	snprintf(full_filename, 256, "%s" OPTIONSFILENAME, file_prefix);
	configfile = fopen(full_filename, "rb");
#else
	configfile = fopen(OPTIONSFILENAME, "rb");
#endif
	if(configfile)
	{
		fread(&num_settings, sizeof num_options_set, 1, configfile);
		for(ii=0; ii<num_settings; ii++)
		{
			fread(&entry, sizeof entry, 1, configfile);
			// TODO: Add a sanity check that this value is legal for the option.
			set_option(entry.key, entry.value);
		}
		fclose(configfile);
	}
}
//}}}
//{{{
void save_options(void)
{
	FILE *configfile;
#ifdef REALCOMPUTER
	char full_filename[256];
#endif
#ifdef IS_CALCULATOR
	EM_moveSymFromExtMem(SYMSTR_CONST(OPTIONSFILENAME), HS_NULL);
#endif
	
#ifdef REALCOMPUTER
	snprintf(full_filename, 256, "%s" OPTIONSFILENAME, file_prefix);
	configfile = fopen(full_filename, "wb");
#else
	configfile = fopen(OPTIONSFILENAME, "wb");
#endif
	
	if(!configfile) {
		message("Couldn't open configuration file to save options.");
		return;
	}
	
	fwrite(&num_options_set, sizeof(ulong), 1, configfile);
	fwrite(options_set, sizeof(option_value), num_options_set, configfile);
	fclose(configfile);
	
#ifdef IS_CALCULATOR
	if(get_option(OPTION_ARCHIVEOPTIONS) == OPTION_ARCHIVE_YES)
		archive_file(OPTIONSFILENAME, sizeof(configfile));
#endif
}
//}}}

