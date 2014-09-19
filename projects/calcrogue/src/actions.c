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
// actions.c
/// @file
/// @brief Code to handle the 'actions menu', a pull-up list of all supported
/// commands.

#include "crogue.h"
#include "export.h"

/// An entry in the 'actions' menu
typedef struct actions_menu_entry
{
	const char *str;  ///< Description of the command
	int key;          ///< Key code which triggers the command
} actions_menu_entry;

static const actions_menu_entry actions[] =
	{
		{ gettext("Walk left"),                  KEY_WEST                   },
		{ gettext("Run left"),                   KEY_RUN_WEST               },
#ifdef KEY_SCROLLING
		{ gettext("Scroll view left"),           KEY_SCROLL_LEFT            },
#endif
		{ gettext("Wait"),                       KEY_WAIT                   },
		{ gettext("Climb stairs"),               KEY_STAIR                  },
		{ gettext("Show inventory"),             KEY_INVENTORY              },
		{ gettext("Fire readied ammo"),          KEY_FIRE                   },
		{ gettext("Cast spell"),                 KEY_CAST                   },
		{ gettext("Use item"),                   KEY_USE                    },
		{ gettext("Equip item"),                 KEY_WEAR                   },
		{ gettext("Equip items"),                KEY_WEAR_MULTIPLE          },
		{ gettext("Take off item"),              KEY_TAKEOFF                },
		{ gettext("Take off items"),             KEY_TAKEOFF_MULTIPLE       },
		{ gettext("Drop item"),                  KEY_DROP                   },
		{ gettext("Drop items"),                 KEY_DROP_MULTIPLE          },
		{ gettext("Pick up items"),              KEY_PICKUP                 },
		{ gettext("Throw item"),                 KEY_THROW                  },
		{ gettext("Show message history"),       KEY_MESSAGE_HISTORY        },
		{ gettext("Show known item types"),      KEY_DISCOVERIES            },
		{ gettext("Search"),                     KEY_SEARCH                 },
		{ gettext("Open door"),                  KEY_OPENDOOR               },
		{ gettext("Close door"),                 KEY_CLOSEDOOR              },
		{ gettext("Options"),                    KEY_OPTIONS_MENU           },
		{ gettext("Help"),                       KEY_HELPMENU               },
		{ gettext("Character info"),             KEY_STATS                  },
		{ gettext("About"),                      KEY_ABOUT                  },
#ifdef ALLOWDEBUG
		{ gettext("Debug"),                      KEY_DEBUG                  },
#endif
#ifdef SUPPORT_HOTKEY_ITEM
		{ gettext("Hotkey item"),                KEY_HOTKEY_ITEM            },
#endif
		{ gettext("Redraw screen"),              KEY_REDRAW                 },
		{ gettext("Quit"),                       KEY_ESC                    },
		{ gettext("Save game"),                  KEY_SAVEGAME               },
		{ NULL, 0 }
	};

static void actions_menu_callback(int n, char *buf)
{
	sprintf(buf, "%s\t%s", actions[n].str, get_keyname(actions[n].key));
}

/// @brief Count the entries in the actions table, using the fact that the
/// last entry is null
static int num_actions(void)
{
	int ii;
	for(ii=0; ; ii++) {
		if(actions[ii].str == NULL)
			return ii;
	}
}

/// Display a menu of possible actions
int actions_menu(void)
{
	int ret;
	setTabStops(actions_menu_tab_stops);
	ret = UI_Menu_Pick( actions_rect, num_actions(), &actions_menu_callback );
	full_redraw();
	if(ret<0)
		return KEY_NOP;
	return actions[ret].key;
}


