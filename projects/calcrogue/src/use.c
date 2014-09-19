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
// use.c
/// @file
/// @brief Use an item

#include "crogue.h"

//{{{
void use_item(void)
{
	int which;
	int itemtype;
	
	if( filter_matches(filter_usable) == 0 )
	{
		message(gettext("You don't have anything you can use."));
		return;
	}
	message(gettext("Use what?"));
	which = pick_item(filter_usable);
	
	if(which<0) {
		message(gettext("Never mind."));
		return;
	}
	
	itemtype  = INVENTORY(which).type;
	
	if(get_item_property(&INVENTORY(which), PROPERTY_USE_EFFECT))
	{
		if(get_item_property(&INVENTORY(which), PROPERTY_USE_EFFECT)
		 && call_usefunc( get_item_property(&INVENTORY(which),
			PROPERTY_USE_EFFECT)->function, which ))
		{
			identify_type(itemtype);
		}
		
		timepass(1);
		return;
	} else {
		message(gettext("You can't use that."));
	}
}
//}}}
