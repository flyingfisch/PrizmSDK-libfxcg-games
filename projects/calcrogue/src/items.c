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
// items.c
/// @file
/// @brief Functions handling placement, use and manipulation of items.

#include "crogue.h"
#include "export.h"

// Inventory menus
static int items_menu(inventoryfilter f);
static void itemstring(int n, char *buf);
static int item_by_sequence(int n);
static void sort_inventory(void);

// Equipment handling
static int num_player_equipment(int slot);

// Utility functions
static void append_armor_description(const item *itm, char *output);
static const item_property *extract_item_property(const item_properties *pr, uint type);
static ulong item_value(const item *i);

// Item storage
static uint add_item(item itm);
static void delete_item(uint which);

static schar random_plus(void);
static uint can_stack(const item *one, const item *two);

// Player actions
static void do_drop_item(int which);
static void do_wear_item(int which);
static void do_takeoff_item(int which);

/*
 * Constant data
 */
#define NUM_ITEM_CATEGORIES 13
//{{{
static const char *categoryNames[NUM_ITEM_CATEGORIES] =
	{
		gettext("Weapons"),
		gettext("Ammunition"),
		gettext("Armor"),
		gettext("Amulets"),
		gettext("Rings"),
		gettext("Wands"),
		gettext("Potions"),
		gettext("Scrolls"),
		gettext("Spellbooks"),
		gettext("Food"),
		gettext("Gems"),
		gettext("Money"),
		gettext("Tools")
	};
//}}}

/*
 * Inventory filters
 */
//{{{
int filter_none(const item *i)
{
	return 1;
}
//}}}
//{{{
int filter_cursed(const item *i)
{
	return (i->flags & ITEMFLAG_CURSED) && (i->flags & ITEMFLAG_IDENTIFIED);
}
//}}}
//{{{
int filter_worn(const item *i)
{
	return i->flags & ITEMFLAG_EQUIPPED;
}
//}}}
//{{{
int filter_wearable(const item *i)
{
	if(i->flags & ITEMFLAG_EQUIPPED)
		return 0;
	return ITEMDESC(*i).spot >= 0;
}
//}}}
//{{{
int filter_unidentified(const item *i)
{
	itemdesc *desc = &ITEMDESC(*i);
	
	if(!type_is_identified(i->type))
		return 1;
	else if(!(i->flags & ITEMFLAG_IDENTIFIED))
	{
		// An individual item of identified type must be (potentially) special in 
		// some way to be considered unidentified
		if(desc->flags & ITEMFLAG_PLUS)
			return 1;
		if((desc->flags & ITEMFLAG_CHARGED) && !(desc->flags & ITEMFLAG_HIDECHARGE))
			return 1;
	}
	return 0;
}
//}}}
//{{{
int filter_usable(const item *i)
{
	return (ITEMDESC(*i).flags & ITEMFLAG_USABLE);
}
//}}}
//{{{
int filter_enchantable(const item *i)
{
	return ITEMDESC(*i).flags & (ITEMFLAG_PLUS | ITEMFLAG_CHARGED);
}
//}}}
//{{{
int filter_plural(const item *i)
{
	return (i->stacksize != 1 && ITEMDESC(*i).flags & ITEMFLAG_STACK
		&& !(ITEMDESC(*i).flags & ITEMFLAG_HIDECHARGE));
}
//}}}
//{{{
int filter_rusty(const item *i)
{
	return (i->rustiness > 0);
}
//}}}

/*
 * Inventory menus
 */
static inventoryfilter invfilter;

//{{{
int pick_item(inventoryfilter f)
{
	int ret;
	ret = items_menu(f);
	full_redraw();
	return ret;
}
//}}}
//{{{
void pick_multiple(inventoryfilter f, void (*function)(int which))
{
	int which;
	
	UI_Menu_Set_Persist(1);
	
	do
	{
		which = items_menu(f);
		if(which >= 0)
			function(which);
	}
	while(which >= 0);
	
	UI_Menu_Set_Persist(0);
	full_redraw();
}
//}}}
//{{{
static int items_menu(inventoryfilter f)
{
	int ret;
	int num_choices;
	
	if(count_inventory()==0)
		return -2;
	
	sort_inventory();
	
	invfilter = f;
	num_choices = filter_matches(f);
	
	if(!num_choices)
		return -2;
	
	setTabStops(item_tab_stops);
	ret = UI_Menu_Pick( inventory_rect, num_choices, &itemstring );
	
	if(ret<0)
		return -1;
	
	ret = item_by_sequence(ret);
	
	if( ret<0 )
		return -1;
	else
		return ret;
}
//}}}
//{{{
static void itemstring(int n, char *buf)
{
	static int prev_n = -2;
	static int i = 0;
	static int prev_category;
	char hotkey_display;
	
	if(n == prev_n+1)
	{
		prev_n = n;
		n=1;
	} else {
		prev_n = n;
		prev_category = -1;
		i=0;
	}
	
	for(; i<PLRINVSIZE; i++)
	{
		if( !invfilter(&INVENTORY(i)) )
			continue;
		
		if(ITEMDESC(INVENTORY(i)).category != prev_category)
		{
			if(!n)
			{
				sprintf(buf, "\n%s",
					categoryNames[ITEMDESC(INVENTORY(i)).category]);
				break;
			}
			n--;
			
			prev_category = ITEMDESC(INVENTORY(i)).category;
		}
		
		if(!n)
		{
			hotkey_display = INVENTORY(i).hotkey;
#ifdef REALCOMPUTER
			if(hotkey_display)
				sprintf(buf,"%c - %s",hotkey_display,itemname(&INVENTORY(i)));
			else
				strcpy(buf, itemname(&INVENTORY(i)));
#else
			if(!hotkey_display) hotkey_display = ' ';
			sprintf(buf, "%c\t%s", hotkey_display, itemname(&INVENTORY(i)));
#endif
			break;
		}
		n--;
	}
}
//}}}
//{{{
// Works ONLY on a pre-sorted inventory. (use sort_inventory())
static int item_by_sequence(int n)
{
	int i;
	int prev_category = -1;
	
	for(i=0; i<PLRINVSIZE; i++)
	{
		if( !invfilter(&INVENTORY(i)) )
			continue;
		
		if(ITEMDESC(INVENTORY(i)).category != prev_category)
		{
			if(!n)
				return -ITEMDESC(INVENTORY(i)).category - 1;
			n--;
			
			prev_category = ITEMDESC(INVENTORY(i)).category;
		}
		
		if(!n)
			return i;
		n--;
	}

	return 0;
}
//}}}
//{{{
static void sort_inventory(void)
{
	int i, j;
	int num=0;
	int cont;
	item tmp;
	
	for(i=0; i<NUM_ITEM_CATEGORIES; i++)
		for(j=0; j<PLRINVSIZE; j++)
			if(INVENTORY(j).type && ITEMDESC(INVENTORY(j)).category == i)
				INVENTORY(j).next = num++;
	
	do
	{
		cont = 0;
		
		for(i=0; i<PLRINVSIZE; i++)
			if(INVENTORY(i).type && INVENTORY(i).next != i)
			{
				cont = 1;
				
				tmp = INVENTORY(INVENTORY(i).next);
				INVENTORY(INVENTORY(i).next) = INVENTORY(i);
				INVENTORY(i) = tmp;
			}
	}
	while(cont);
}
//}}}

//{{{
int filter_matches(inventoryfilter f)
{
	int i, j;
	int n=0;
	
	for(i=0; i<PLRINVSIZE; i++)
		if(INVENTORY(i).type && f(&INVENTORY(i)))
			n++;
	
	for(i=0; i<NUM_ITEM_CATEGORIES; i++)
		for(j=0; j<PLRINVSIZE; j++)
			if(INVENTORY(j).type && ITEMDESC(INVENTORY(j)).category == i && f(&INVENTORY(j)))
			{
				n++;
				break;
			}
	
	return n;
}
//}}}

/*
 * Player inventory
 */
//{{{
void init_inventory(void)
{
	const uint *ptr;
	const item *contents;
	uint i;
	uint num_items;
	
	// Initialize inventory empty
	memset(&w->plr.inventory, 0, sizeof(item) * PLRINVSIZE);
	
	ptr = (const uint*)deref_file_ptr(w->desc.startinginventory);
	num_items = *ptr;
	contents = (item*)(ptr+1);
	
	for(i=0; i<num_items; i++)
	{
		INVENTORY(i) = contents[i];
		
		identify_type(INVENTORY(i).type);
	}
}
//}}}
//{{{
item split_item(int slot)
{
	item ret = INVENTORY(slot);
	
	if(!(ITEMDESC(INVENTORY(slot)).flags & ITEMFLAG_STACK))
	{
		INVENTORY(slot).type = 0;
		return ret;
	}
	
	ret.stacksize = 1;
	
	INVENTORY(slot).stacksize--;
	if(INVENTORY(slot).stacksize<=0)
	{
		INVENTORY(slot).type = 0;
	}
	
	return ret;
}
//}}}
//{{{
void merge_item(int slot, item it)
{
	if(INVENTORY(slot).type == 0) {
		INVENTORY(slot) = it;
	} else {
		INVENTORY(slot).stacksize += it.stacksize;
	}
}
//}}}
//{{{
int give_item(const item *itm, sint silent)
{
	int i;
	
	// Look for items of the same type to stack with
	if(ITEMDESC(*itm).flags & ITEMFLAG_STACK) {
		for(i=0; i<PLRINVSIZE; i++) {
			if( can_stack(&INVENTORY(i), itm) )
			{
				INVENTORY(i).stacksize += itm->stacksize;
				if( !silent ) {
					message(gettext("You have %s."), itemname(&INVENTORY(i)));
				}
				return 1;
			}
		}
	}
	// If it can't stack, look for empty slots
	for(i=0; i<PLRINVSIZE; i++) {
		if(!INVENTORY(i).type) {
			if( !silent ) {
				message(gettext("You got the %s."), itemname(itm) );
			}
			INVENTORY(i) = *itm;
			return 1;
		}
	}
	// Otherwise, give up
	message(gettext("You don't have room in your pack for the %s."),
		itemname(itm));
	return 0;
}
//}}}
//{{{
void give_item_slip(const item *itm)
{
	int i;
	
	// Look for items of the same type to stack with
	if(ITEMDESC(*itm).flags & ITEMFLAG_STACK) {
		for(i=0; i<PLRINVSIZE; i++) {
			if( can_stack(&INVENTORY(i), itm) )
			{
				INVENTORY(i).stacksize += itm->stacksize;
				message(gettext("You have %s."), itemname(&INVENTORY(i)));
				return;
			}
		}
	}
	// If it can't stack, look for empty slots
	for(i=0; i<PLRINVSIZE; i++) {
		if(!INVENTORY(i).type) {
			message(gettext("You got the %s."), itemname(itm) );
			INVENTORY(i) = *itm;
			return;
		}
	}
	// Otherwise, drop it on the floor
	place_item(itm, w->plr.x, w->plr.y);
	if(filter_plural(itm))
		message(gettext("The %s slips from your grasp."), itemname(itm));
	else
		message(gettext("The %s slip from your grasp."), itemname(itm));
}
//}}}

/*
 * Equipment handling
 */
//{{{
item *player_equipment(int slot)
{
	int i;
	
	for(i=0; i<PLRINVSIZE; i++)
	{
		if(INVENTORY(i).type && ITEMDESC(INVENTORY(i)).spot == slot && INVENTORY(i).flags & ITEMFLAG_EQUIPPED)
			return &INVENTORY(i);
	}
	
	return NULL;
}
//}}}
//{{{
static int num_player_equipment(int slot)
{
	int i;
	int n=0;
	
	for(i=0; i<PLRINVSIZE; i++)
	{
		if(INVENTORY(i).type && ITEMDESC(INVENTORY(i)).spot == slot && INVENTORY(i).flags & ITEMFLAG_EQUIPPED)
			n++;
	}
	return n;
}
//}}}

/*
 * Utility functions
 */

//{{{
uint count_inventory(void)
{
	int i, num=0;
	for(i=0; i<PLRINVSIZE; i++)
		if(INVENTORY(i).type)
			num++;
	return num;
}
//}}}
//{{{
int iterate_item_property(const item_properties *pr, uint type, short (*func)(const item_property *pr))
{
	int i;
	const item_property *pos = &pr->properties[0];
	
	for(i=0; i<pr->num_properties; i++)
	{
		if(pos->type == PROPERTY_INHERIT)
		{
			if( iterate_item_property( (const item_properties*)deref_file_ptr
				(pos->contents.parent), type, func ) )
				return 1;
		}
		else if(type == pos->type)
		{
			if( func(pos) )
				return 1;
		}
		pos++;
	}
	return 0;
}
//}}}
//{{{
const item_property_desc *get_item_property(const item *itm, uint type)
{
	const item_property *ret = extract_item_property( &ITEMDESC(*itm).properties, type );
	if(!ret)
		return NULL;
	return &ret->contents;
}
//}}}
//{{{
static const item_property *extract_item_property(const item_properties *pr, uint type)
{
	int i;
	const item_property *pos = &pr->properties[0], *ret;
	
	for(i=0; i<pr->num_properties; i++)
	{
		if(pos->type == PROPERTY_INHERIT)
		{
			if( (ret = extract_item_property( (const item_properties*)
				deref_file_ptr(pos->contents.parent), type )) )
				return ret;
		}
		else if(type == pos->type)
			return pos;
		pos++;
	}
	return NULL;
}
//}}}
static const item *current_item;
static sint *current_target;
static uint current_stat;

//{{{
static short cb_apply_item_stat(const item_property *pr)
{
	const stat_mod_desc *s = &pr->contents.worn_stat_effect;
	if(s->stat != current_stat)
		return 0;
	
	apply_stat_mod_desc(current_target, s, current_item->plus);
	return 0;
}
//}}}
//{{{
static short cb_apply_item_stat_all(const item_property *pr)
{
	const stat_mod_desc *s = &pr->contents.worn_stat_effect;
	
	apply_stat_mod_desc(NULL, s, current_item->plus);
	return 0;
}
//}}}
//{{{
void apply_item_property(const item *itm, uint stat, sint *n)
{
	current_item = itm;
	current_target = n;
	current_stat = stat;
	iterate_item_property(&ITEMDESC(*itm).properties, PROPERTY_WORN_STAT_EFFECT, &cb_apply_item_stat);
}
//}}}
//{{{
void apply_all_item_properties(const item *itm, int trigger)
{
	current_item = itm;
	iterate_item_property(&ITEMDESC(*itm).properties, trigger, &cb_apply_item_stat_all);
}
//}}}

//{{{
const char *shortitemname(const item *i)
{
	itemdesc *desc;
	desc = &ITEMDESC(*i);
	if( type_is_identified(i->type) || isNull(desc->unidname)
		|| i->flags & ITEMFLAG_UNPAID )
		return (uchar*)deref_file_ptr(desc->name);
	else
		return ((const itemextension*)deref_file_ptr(desc->unidname))->str;
}
//}}}

//{{{
const char *rusty_descriptions[] =
	{
		gettext("pristine "),
		gettext("slightly rusty "),
		gettext("rusty "),
		gettext("very rusty ")
	};
//}}}
//{{{
uchar *n_itemname(ulong which)
{
	return itemname(&INVENTORY(which));
}
//}}}
//{{{
uchar *itemname(const item *i)
{
	const char *n;
	long count;
	static char name[128];
	uint identified;
	itemdesc *desc;
	
	identified = i->flags & ITEMFLAG_IDENTIFIED;
	desc = &ITEMDESC(*i);
	
	n = shortitemname(i);
	
	strcpy(name, "");
	
	if(filter_plural(i))
		catprintf(name, "%li ", i->stacksize);
	if(i->flags & ITEMFLAG_CURSED && identified)
		catprintf(name, gettext("cursed "));
	
	if(i->rustiness > 0)
		catprintf(name, rusty_descriptions[i->rustiness]);
	
	if(!(ITEMDESC(*i).flags & ITEMFLAG_STACK))
		count=1;
	else if(ITEMDESC(*i).flags & ITEMFLAG_HIDECHARGE)
		count=1;
	else
		count = i->stacksize;
	
	catprintf(name, pluralize(count, n));
	
	if(desc->flags & ITEMFLAG_CHARGED && !(desc->flags & ITEMFLAG_HIDECHARGE) && identified)
		catprintf(name, gettext(" (%li charges)"), i->stacksize);
	
	if(identified)
	{
		append_armor_description(i, name);
	}
	if(i->flags & ITEMFLAG_EQUIPPED)
	{
		if(desc->spot == 0)
			catprintf(name, gettext(" (wielded)"));
		else if(desc->spot == 1)
			catprintf(name, gettext(" (readied)"));
		else catprintf(name, gettext(" (worn)"));
	}
	
	if(i->flags & ITEMFLAG_UNPAID)
		catprintf(name, gettext(" (%li gold)"), item_cost(i));
	
	return name;
}
//}}}

//{{{
typedef struct statdisplay
{
	uint stat;
	const char *str;
} statdisplay;

#define NUM_DISPLAYED_STATS 7
const statdisplay displayed_stats[NUM_DISPLAYED_STATS] = {
	{STAT_STRENGTH, gettext("St")},
	{STAT_DEXTERITY, gettext("Dx")},
	{STAT_TOUGHNESS, gettext("To")},
	{STAT_MANA, gettext("Ma")},
	{STAT_WILLPOWER, gettext("Wi")},
	{STAT_LIGHTRADIUS, gettext("Light")},
	{STAT_SPEED, gettext("Spd")}
	};
//}}}
static sint tmp_stats[NUM_STATS];
//{{{
static short cb_apply_item_stat_to_temp(const item_property *pr)
{
	apply_stat_mod_desc(&tmp_stats[pr->contents.worn_stat_effect.stat], &pr->contents.worn_stat_effect, current_item->plus);
	return 0;
}
//}}}
//{{{
static void append_armor_description(const item *itm, char *output)
{
	int i;
	current_item = itm;
	
	for(i=0; i<NUM_STATS; i++)
		tmp_stats[i] = 0;
	
	iterate_item_property(&ITEMDESC(*itm).properties, PROPERTY_WORN_STAT_EFFECT, &cb_apply_item_stat_to_temp);
	iterate_item_property(&ITEMDESC(*itm).properties, PROPERTY_THROWN_EFFECT, &cb_apply_item_stat_to_temp);
	if(tmp_stats[STAT_MISSILEDAM_MAX] > tmp_stats[STAT_DAMAGE_MAX])
		tmp_stats[STAT_DAMAGE_MAX] = tmp_stats[STAT_MISSILEDAM_MAX];
	if(tmp_stats[STAT_MISSILEDAM_MIN] > tmp_stats[STAT_DAMAGE_MIN])
		tmp_stats[STAT_DAMAGE_MIN] = tmp_stats[STAT_MISSILEDAM_MIN];
	if(tmp_stats[STAT_MISSILE_TOHIT] > tmp_stats[STAT_TOHIT])
		tmp_stats[STAT_TOHIT] += tmp_stats[STAT_MISSILE_TOHIT];
	
	if(tmp_stats[STAT_DV] || tmp_stats[STAT_PV])
		catprintf(output, " [%i, %i]", tmp_stats[STAT_DV], tmp_stats[STAT_PV]);
	if(tmp_stats[STAT_DAMAGE_MIN] || tmp_stats[STAT_DAMAGE_MAX] || tmp_stats[STAT_TOHIT]) {
		catprintf(output, " (");
		
		if(tmp_stats[STAT_DAMAGE_MIN] || tmp_stats[STAT_DAMAGE_MAX]) {
			catprintf(output, "%i-%i", tmp_stats[STAT_DAMAGE_MIN], tmp_stats[STAT_DAMAGE_MAX]);
			if(tmp_stats[STAT_TOHIT])
				catprintf(output, ", ");
		}
		
		if(tmp_stats[STAT_TOHIT] > 0)
			catprintf(output, "+%i)", tmp_stats[STAT_TOHIT]);
		else if(tmp_stats[STAT_TOHIT] < 0)
			catprintf(output, "%i)", tmp_stats[STAT_TOHIT]);
		else
			catprintf(output, ")");
	}
	
	for(i=0; i<NUM_DISPLAYED_STATS; i++)
	{
		if(tmp_stats[displayed_stats[i].stat])
		{
			if(tmp_stats[displayed_stats[i].stat] > 0)
				catprintf(output, " (+%i %s)", tmp_stats[displayed_stats[i].stat], displayed_stats[i].str);
			else
				catprintf(output, " (%i %s)", tmp_stats[displayed_stats[i].stat], displayed_stats[i].str);
		}
	}

}
//}}}

//{{{
void award_item_points(void)
{
	int i;
	const item_property_desc *factor;
	for(i=0; i<PLRINVSIZE; i++)
	{
		if(INVENTORY(i).type==0)
			continue;
		factor = get_item_property(&INVENTORY(i), PROPERTY_SCORE);
		if(factor)
			w->plr.score += factor->score_factor * ITEMDESC(INVENTORY(i)).value * INVENTORY(i).stacksize;
	}
}
//}}}
//{{{
void rust_inventory(uint which)
{
	if(ITEMDESC(INVENTORY(which)).flags & ITEMFLAG_RUSTPROOF)
		return;
	
	switch(INVENTORY(which).rustiness)
	{
		case 0:
			message(gettext("Your %s rusts!"), shortitemname(&INVENTORY(which)));
			INVENTORY(which).rustiness = 1;
			INVENTORY(which).plus --;
			break;
		case 1:
		case 2:
			message(gettext("Your %s rusts further!"), shortitemname(&INVENTORY(which)));
			INVENTORY(which).rustiness++;
			INVENTORY(which).plus--;
			break;
		case 3:
			message(gettext("Your %s collapses into a pile of rust."), shortitemname(&INVENTORY(which)));
			INVENTORY(which).type = 0;
			break;
	}
}
//}}}
//{{{
// Returns:
//   0: Fixed an item
//   1: Nothing to fix
//   2: Player cancelled
//   3: Not enough money or refused
ushort fix_rust(void)
{
	long cost;
	int which;

	if(filter_matches(filter_rusty) == 0)
		return 1;
	
	which = pick_item(filter_rusty);
	
	if(which == -1)
		return 2;
	
	cost = INVENTORY(which).rustiness * 100;
	
	if(prompt(retprintf(gettext("Fix your %s for %li gold?"), itemname(&INVENTORY(which)), cost)))
	{
		if(player_gold() < cost) {
			message(gettext("You don't have enough money."));
			return 3;
		}
		debit_gold(cost);
		message(gettext("Your %s is fixed."), itemname(&INVENTORY(which)));
		INVENTORY(which).plus += INVENTORY(which).rustiness;
		INVENTORY(which).rustiness = 0;
	}
	return 3;
}
//}}}

/*
 * Shopping
 */
//{{{
void give_gold(ulong amt)
{
	struct item gold;
	
	gold = randitem(w->desc.generate_currency);
	gold.stacksize = amt;
	
	give_item(&gold, 1);
}
//}}}
//{{{
ulong player_gold(void)
{
	int i;
	for(i=0; i<PLRINVSIZE; i++)
	{
		if(ITEMDESC(INVENTORY(i)).flags & ITEMFLAG_CURRENCY)
			return INVENTORY(i).stacksize;
	}
	return 0;
}
//}}}
//{{{
void debit_gold(ulong amt)
{
	int i;
	for(i=0; i<PLRINVSIZE; i++)
	{
		if(ITEMDESC(INVENTORY(i)).flags & ITEMFLAG_CURRENCY)
		{
			if(INVENTORY(i).stacksize > amt)
				INVENTORY(i).stacksize -= amt;
			else
				INVENTORY(i).type = 0;
			return;
		}
	}
}
//}}}

//{{{
static ulong item_value(const item *i)
{
	int v;
	int factor = 5;
	int mod = 0;
	
	v = ITEMDESC(*i).value;
	if(i->plus <= -2)      factor = 2;
	else switch(i->plus) {
		case -1: factor = 4; break;
		case 0: factor = 5; break;
		case 1: factor = 6; mod = 20; break;
		case 2: factor = 7; mod = 50; break;
		case 3: factor = 8; mod = 100; break;
		case 4: factor = 9; mod = 200; break;
		case 5: factor = 10; mod = 300; break;
	}
	v = (v*factor)/5;
	if(ITEMDESC(*i).flags & ITEMFLAG_STACK)
		v *= i->stacksize;
	else if(ITEMDESC(*i).flags & ITEMFLAG_CHARGED && i->stacksize>1)
		v += (ITEMDESC(*i).value) * (i->stacksize-1) / 15;
	else
		v += mod;
	return v;
}
//}}}
//{{{
ulong item_cost(const item *i)
{
	if(!(i->flags & ITEMFLAG_UNPAID))
		return 0;
	
	return item_value(i);
}
//}}}
//{{{
ulong sale_price(const item *i)
{
	return (item_value(i) * 1l) / 3l;
}
//}}}

/*
 * Misc
 */
//{{{
void list_discoveries(void)
{
	int i;
	int num_discoveries = 0;
	char buf[256];
	draw_string_info state = {0, 0};
	
	setTabStops(discovery_screen_tab_stops);
	
	for(i=0; i<w->desc.itementries; i++)
	{
		if(type_is_identified(i) && !isNull(ITEMDESCN(i).unidname))
		{
			if(!num_discoveries)
				Graph_ClrScr();
			
			sprintf(buf, "%s\t%s\n", ((const itemextension*)deref_file_ptr(ITEMDESCN(i).unidname))->str,
			                       (const char*)deref_file_ptr(ITEMDESCN(i).name));
#ifdef IS_CALCULATOR
			SetFont(OPTION_FONT_SMALL);
#endif
			draw_string(buf, &state, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
			num_discoveries++;
		}
	}
	if(!num_discoveries)
	{
		message(gettext("You haven't identified anything interesting."));
		return;
	}
	read_char();
	full_redraw();
}
//}}}
//{{{
void init_discoveries(void)
{
	int i;
	w->itemids = (ulong*)debug_malloc( (w->desc.itementries/32 + 1) * 4 );
	for(i=0; i < w->desc.itementries/32 + 1; i++)
		w->itemids[i] = 0;
	for(i=0; i<w->desc.itementries; i++) {
		if( ITEMDESCN(i).flags & ITEMFLAG_AUTOID )
			identify_type(i);
	}
}
//}}}
//{{{
void identify_type(ushort t)
{
	w->itemids[t/32] |= 1l << (t%32l);
}
//}}}
//{{{
ulong type_is_identified(ulong t)
{
	return w->itemids[t/32] & (1l << (t % 32));
}
//}}}

/*
 * Item storage
 */
static const int items_startsize = 64;
static const int items_allocinterval = 8;

//{{{
void compress_items(void)
{
	int i;
	
	// Zero out unused items
	for(i=0; i<w->items.num; i++)
	{
		if(!w->items.items[i].type)
			memset(&w->items.items[i], 0, sizeof(item));
	}
}
//}}}
//{{{
void place_item(const item *i, ushort x, ushort y)
{
	int top = top_item(x, y);
	int pos;
	
	// Check for items on the floor which it can stack with
	item itm = *i;
	
	pos = top;
	
	while(pos)
	{
		if(can_stack(&itm, &w->items.items[pos]))
		{
			w->items.items[pos].stacksize += itm.stacksize;
			return;
		}
		pos = w->items.items[pos].next;
	}

	// Add to the top of the stack
	w->t[y][x].flags |= TFLAG_ITEM;
	if(top) w->items.items[top].flags &= ~ITEMFLAG_TOP;
	itm.next = top;
	itm.flags |= ITEMFLAG_TOP;
	itm.x = x;
	itm.y = y;
	add_item(itm);
}
//}}}
//{{{
uchar draw_item(uchar x, uchar y)
{
	return ITEMDESC( w->items.items[ top_item(x,y) ] ).drawchar;
}
//}}}
//{{{
short count_items(ushort x, ushort y)
{
	int pos, ret;
	
	if(!(w->t[y][x].flags & TFLAG_ITEM))
		return 0;
	
	pos = top_item(x, y);
	ret = 0;
	
	while(pos != 0) {
		pos = w->items.items[pos].next;
		ret++;
	}
	return ret;
}
//}}}
//{{{
short top_item(ushort x, ushort y)
{
	int ii, best_try = 0;
	item *itm;
	
	if(!(w->t[y][x].flags & TFLAG_ITEM))
		return 0;
	
	for(ii=0; ii<w->items.alloced; ii++)
	{
		itm = &w->items.items[ii];
		if(itm->x == x && itm->y == y)
		{
			if(itm->flags & ITEMFLAG_TOP)
				return ii;
			else // In case of corruption, at least get something in the chain
				best_try = ii;
		}
	}
#ifdef DEBUG_HEAVILY
	message(gettext("WARNING: Damaged item chain at (%i,%i)"), (int)x, (int)y);
#endif
	return best_try;
}
//}}}
#ifdef SUPPORT_COLOR
//{{{
colorinfo color_item(uchar x, uchar y)
{
	item *i = &w->items.items[top_item(x,y)];
	colorinfo ret = {7|COLOR_BOLD, 7|COLOR_BOLD};
	
	if(ITEMDESC(*i).color.lit)
		ret = ITEMDESC(*i).color;
	else if(!isNull(ITEMDESC(*i).unidname))
		ret = ((const itemextension*)deref_file_ptr(ITEMDESC(*i).unidname))->color;
	return ret;
}
//}}}
#endif
//{{{
void init_items()
{
	int i;
	
	w->items.num = 0;
	w->items.alloced = items_startsize;
	
	w->items.items = debug_malloc(sizeof(item) * items_startsize);
	
	for(i=0; i<w->items.alloced; i++)
		w->items.items[i].type = 0;
}
//}}}
//{{{
void cleanup_items(void)
{
	if(w->items.items)
		debug_free(w->items.items);
}
//}}}
//{{{
static uint add_item(item itm)
{
	int i;
	
	// Allocate more memory if necessary
	w->items.num++;
	if(w->items.num >= w->items.alloced) {
		w->items.alloced += items_allocinterval;
		w->items.items = (item*)debug_realloc(w->items.items,
			sizeof(item) * w->items.alloced);
		
		// Init newly created space
		for(i=w->items.alloced - items_allocinterval; i<w->items.alloced; i++)
			w->items.items[i].type = 0;
	}
	
	// Find an empty spot in the items array
	// i<items_alloced test unnecessary since resizing guarantees break first
	// Starting at 1 (wasting spot 0) to avoid confusing item 0 with 'no item'
	for(i=1; ; i++) {
		if(!w->items.items[i].type)
			break;
	}
	w->items.items[i] = itm;
	
	return i;
}
//}}}
//{{{
static void delete_item(uint which)
{
	int i;

	// Find any items with link-fields pointing to the deleted item and redirect
	for(i=0; i<w->items.alloced; i++)
	{
		if(w->items.items[i].next == which)
			w->items.items[i].next = w->items.items[which].next;
	}
	w->items.items[which].type = 0;
	w->items.num--;
}
//}}}

//{{{
item randitem(filelink type)
{
	item i;
	memset(&i, 0, sizeof(i));
	
	i.type = ((itemdesc*)deref_file_ptr(type)) -> num;
	
	i.stacksize = 1;
	i.rustiness = 0;
	
	if(ITEMDESC(i).flags & (ITEMFLAG_STACK|ITEMFLAG_CHARGED|ITEMFLAG_FUELED|ITEMFLAG_HIDECHARGE))
		i.stacksize = nrandom(ITEMDESC(i).stacksize, ITEMDESC(i).stacksize/2);
	if(i.stacksize<1) i.stacksize = 1;
	
	if(ITEMDESC(i).flags & ITEMFLAG_PLUS)
		i.plus = random_plus();
	if(!(ITEMDESC(i).flags & ITEMFLAG_RUSTPROOF))
	{
		if(RANGE(7,0) == 0)
		{
			i.rustiness = RANGE(2,1);
			i.plus -= i.rustiness;
		}
	}
	
	if(i.plus < 0 || ITEMDESC(i).flags & ITEMFLAG_GEN_CURSED)
		i.flags |= ITEMFLAG_CURSED;
	
	return i;
}
//}}}
//{{{
static schar random_plus(void)
{
//{{{
	const schar weighted_plus[16] = {
			1,	1,
			1,	1,
			1,	1,
			1,	1,	
			2,	2,	
			2,	2,	
			3,	3,	
			4,	5	
		};
//}}}
	int t;
	
	// One in 10 items is positively enchanted, another 1 in 10 is negatively enchanted.
	t = RANGE(10,1);
	if     (t==10) return  weighted_plus[lrand()%16];
	else if(t==1)  return -weighted_plus[lrand()%16];
	else           return  0;
}
//}}}

//{{{
static uint can_stack(const item *one, const item *two)
{
	if( !(ITEMDESC(*one).flags & ITEMFLAG_STACK) )
		return 0;
	
	if(one->type != two->type)
		return 0;
	if(one->plus != two->plus)
		return 0;
	if(one->rustiness != two->rustiness)
		return 0;
	
	if( one->plus )
	{
		if( (one->flags ^ two->flags) & ITEMFLAG_IDENTIFIED )
			return 0;
	}
	
	if( (one->flags ^ two->flags)
	    & ~(ITEMFLAG_EQUIPPED|ITEMFLAG_IDENTIFIED|ITEMFLAG_TOP) )
		return 0;
	
	return 1;
}
//}}}

/*
 * Player actions
 */
//{{{
void drop_item(void)
{
	int which;
	
	if( filter_matches(filter_none) == 0 )
	{
		message(gettext("You don't have anything to drop."));
		return;
	}
	
	message(gettext("Drop what?"));
	which = pick_item(filter_none);
	
	if(which<0) {
		message(gettext("Never mind."));
		return;
	}
	
	do_drop_item(which);
}
//}}}
//{{{
void drop_multiple(void)
{
	if( filter_matches(filter_none) == 0 )
	{
		message(gettext("You don't have anything to drop."));
		return;
	}
	message(gettext("Drop what?"));
	
	pick_multiple(filter_none, &do_drop_item);
}
//}}}
//{{{
static void do_drop_item(int which)
{
	ulong value = 0;
	
	if(which<0)
		return;
	
	if(INVENTORY(which).flags & ITEMFLAG_EQUIPPED)
	{
		if( ITEMDESC(INVENTORY(which)).spot == 0 ) {
			message(gettext("You need to unwield it first."));
			return;
		} else {
			message(gettext("You can't drop something you're wearing."));
			return;
		}
	}
	
	if(w->t[w->plr.y][w->plr.x].type == TILE_SHOPFLOOR && !(INVENTORY(which).flags & ITEMFLAG_UNPAID))
	{
		if(ITEMDESC(INVENTORY(which)).flags & ITEMFLAG_CURRENCY) {
			message(gettext("You can't sell gold."));
			return;
		}
		value = sale_price(&INVENTORY(which));
		if(!prompt(retprintf(gettext("Sell your %s for %li gold?"), itemname(&INVENTORY(which)), value)))
			return;
		message(gettext("You sell the %s."), itemname(&INVENTORY(which)));
		INVENTORY(which).flags |= ITEMFLAG_UNPAID;
	} else {
		message(gettext("You drop the %s."), itemname(&INVENTORY(which)));
	}
	place_item(&INVENTORY(which), w->plr.x, w->plr.y);
	INVENTORY(which).type=0;
	if(value)
		give_gold(value);
	
	update_player();
	timepass(1);
}
//}}}
//{{{
static const int max_worn_items[10] =
	{
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		2,
		1
	};
//}}}
//{{{
void wear_item(void)
{
	int which;
	
	if( filter_matches(filter_wearable) == 0 )
	{
		message(gettext("You don't have anything to equip."));
		return;
	}
	message(gettext("Equip what?"));
	
	which=pick_item(filter_wearable);
	
	if(which<0) {
		message(gettext("Never mind."));
		return;
	}
	
	do_wear_item(which);
}
//}}}
//{{{
void wear_multiple(void)
{
	if( filter_matches(filter_wearable) == 0 )
	{
		message(gettext("You don't have anything to equip."));
		return;
	}
	message(gettext("Equip what?"));
	
	pick_multiple(filter_wearable, &do_wear_item);
}
//}}}
//{{{
static void do_wear_item(int which)
{
	int slot;
	item *unequip;
	
	slot = ITEMDESC(INVENTORY(which)).spot;
	
	if(num_player_equipment(slot) >= max_worn_items[slot])
	{
		unequip = player_equipment(slot);
		
		if( unequip )
		{
			if(unequip->flags & ITEMFLAG_CURSED)
			{
				if(slot) message(gettext("You can't remove your old armor."));
				else     message(gettext("Your weapon is welded to your hand!"));
				return;
			}
			unequip->flags &= ~ITEMFLAG_EQUIPPED;
			message(
			    (slot>1)  ? (gettext("You take off the %s")) :
				(slot==1) ? (gettext("You unready the %s"))  :
				            (gettext("You unwield the %s")),
				itemname(unequip));
		}
	}
	
	identify_type(INVENTORY(which).type);
	INVENTORY(which).flags |= ITEMFLAG_IDENTIFIED;
	
	call_usefunc( get_item_property(&INVENTORY(which),
		PROPERTY_WEAR_FUNC)->function, which );
	
	update_player();
	
	timepass(1);
}
//}}}
//{{{
void takeoff_item(void)
{
	int which;
	
	if( filter_matches(filter_worn) == 0 )
	{
		message(gettext("You don't have anything to unequip."));
		return;
	}
	
	message(gettext("Unequip what?"));
	which = pick_item(filter_worn);
	
	if(which<0)
	{
		message(gettext("Never mind."));
		return;
	}
	
	do_takeoff_item(which);
}
//}}}
//{{{
void takeoff_multiple(void)
{
	if( filter_matches(filter_worn) == 0 )
	{
		message(gettext("You don't have anything to unequip."));
		return;
	}
	message(gettext("Unequip what?"));
	
	pick_multiple(filter_worn, &do_takeoff_item);
}
//}}}
//{{{
static void do_takeoff_item(int which)
{
	int slot = ITEMDESC(INVENTORY(which)).spot;
	
	if( !(INVENTORY(which).flags & ITEMFLAG_EQUIPPED) )
	{
		message(gettext("You aren't wearing/wielding that."));
		return;
	}
	
	if( INVENTORY(which).flags & ITEMFLAG_CURSED )
	{
		message(gettext("You can't. It seems to be cursed."));
		return;
	}
	
	INVENTORY(which).flags &= ~ITEMFLAG_EQUIPPED;
	
	message(
		(slot>1)  ? gettext("You take off the %s.") :
		(slot==1) ? gettext("You unready the %s.") :
		            gettext("You unwield the %s."),
		itemname(&INVENTORY(which)));
	
	update_player();
	
	timepass(1);
}
//}}}
//{{{
void item_step(void)
{
	int num;
	item *itm;
	switch(get_option(OPTION_AUTOPICKUP))
	{
		case OPTION_AUTO_NO:
			num = count_items(w->plr.x, w->plr.y);
			if(num > 5)
				message(gettext("You see many items here."));
			else if(num > 1)
				message(gettext("You see several items here."));
			else {
				itm = &w->items.items[top_item(w->plr.x, w->plr.y)];
				if(filter_plural(itm))
					message(gettext("You see %s here."), itemname(itm));
				else
					message(gettext("You see a %s here."), itemname(itm));
			}
			break;
		case OPTION_AUTO_YES:
			pickup(0);
			break;
		case OPTION_AUTO_PROMPT:
			pickup(1);
			break;
	}
}
//}}}
//{{{
void pickup(uint pickup_prompt)
{
	sshort x=w->plr.x, y=w->plr.y;
	uint pos, next;
	uint top;
	uint do_pickup = 0;
	item *i;
	
	top = top_item(x, y);
	pos = top;
	
	while(pos)
	{
		next = w->items.items[pos].next;
		i = &w->items.items[pos];
		
		if(pickup_prompt || (i->flags & ITEMFLAG_UNPAID))
		{
			do_pickup = prompt(
				retprintf(
					((i->flags&ITEMFLAG_UNPAID)?
						gettext("Buy the %s?") :
						gettext("Pick up the %s?")),
					itemname(&w->items.items[pos])
				) );
		} else {
			do_pickup = 1;
		}
		if( do_pickup )
		{
			if(i->flags & ITEMFLAG_UNPAID)
			{
				if(item_cost(i) > player_gold())
				{
					message(gettext("You can't afford that."));
					return;
				}
				identify_type(i->type);
				debit_gold(item_cost(i));
				i->flags &= ~ITEMFLAG_UNPAID;
			}
			if(give_item(&w->items.items[pos], 0))
			{
				if(pos==top) {
					if(w->items.items[pos].next) {
						top = w->items.items[pos].next;
						w->items.items[ top ].flags |= ITEMFLAG_TOP;
					} else {
						w->t[y][x].flags &= ~TFLAG_ITEM;
					}
				}
				delete_item(pos);
				timepass(1);
			}
		}
		pos = next;
	}
	update_player();
}
//}}}
#ifdef SUPPORT_HOTKEY_ITEM
//{{{
void hotkey_item(void)
{
	int which, hotkey;
	message(gettext("Hotkey which item?"));
	which = pick_item(&filter_none);
	if(w<0)
	{
		message(gettext("Never mind."));
		return;
	}
	message(gettext("Use what hotkey? [0-9]"));
	draw();
	hotkey = read_char();
	if(hotkey<'0' || hotkey>'9')
	{
		if(hotkey==KEY_ESC)
			INVENTORY(which).hotkey = 0;
		else
			message(gettext("Invalid selection."));
	}
	else
		INVENTORY(which).hotkey = hotkey;
}
//}}}
#endif

