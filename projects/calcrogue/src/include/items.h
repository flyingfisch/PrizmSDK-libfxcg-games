// items.h
/// @file
/// @brief Items and inventory

#ifndef ITEMS_H
#define ITEMS_H

//{{{
/// An inventory item
struct item
{
	ushort type;       ///< Type index into #w::itemdescs
	ushort next;       ///< (When on the ground) link to next node
	ulong stacksize;   ///< Number of stacked items or charges
	uchar flags;
	schar plus;        ///< Net enchantment (0 for mundane)
	uchar hotkey;      ///< See #hotkey_item
	uchar rustiness;   ///< How rusty the item is (also affects #item::plus)
	uchar x; uchar y;  ///< (When on the ground) location on the map
	ushort padding;
};
//}}}
//{{{
#define ITEMFLAG_EQUIPPED   1
#define ITEMFLAG_CURSED     2
#define ITEMFLAG_IDENTIFIED 4  /* Item (not necessarily the type) is unIDed */
#define ITEMFLAG_UNPAID     8  /* Owned by a shopkeeper */
#define ITEMFLAG_DEPLETE   16  /* Temp flag: kill a charge when it's time */
#define ITEMFLAG_TOP       32  /* This is the top of a floor stack */
//}}}
//{{{
/// Storage of items on a level
struct itemsinfo
{
	uint num;
	uint alloced;
	struct item *items;
};
//}}}
TYPEDEF(struct item item)
TYPEDEF(struct itemsinfo itemsinfo)

//{{{
struct stat_mod_desc
{
	uchar type;
	uchar stat;
	sint amount;
};
TYPEDEF(struct stat_mod_desc stat_mod_desc)
	/// [stat]+=amount
#define STATMOD_ADD     1
	/// [stat]+=amount + item.plus
#define STATMOD_ADDPLUS 2
	/// [stat]|=amount
#define STATMOD_OR      3
	/// [stat]=amount
#define STATMOD_SET     4
//}}}
//{{{
union item_property_desc
{
	struct filelink parent;
	struct stat_mod_desc worn_stat_effect;
	struct stat_mod_desc carried_stat_effect;
	uint launcher;
	struct filelink function;
	uint score_factor;
};
TYPEDEF(union item_property_desc item_property_desc)
//}}}
//{{{
struct item_property
{
	uint type;
	union item_property_desc contents;
};
TYPEDEF(struct item_property item_property)
//}}}
//{{{
struct item_properties
{
	uint num_properties;
	struct item_property properties[1];
};
TYPEDEF(struct item_properties item_properties)
//}}}
//{{{
struct itemextension
{
#ifdef SUPPORT_COLOR
	struct colorinfo color;
#endif
	char str[1];
};
TYPEDEF(struct itemextension itemextension)
//}}}
//{{{
#define PROPERTY_INHERIT          0 /* Inherit properties from another item */
#define PROPERTY_WORN_STAT_EFFECT 1 /* When equipped, apply stat_mod_desc */
#define PROPERTY_WEAR_FUNC        2 /* Function to call when equipping */
#define PROPERTY_LAUNCHER         3 /* Bit-mask of launchers that can be used
                                     *   with this ammo */
#define PROPERTY_USE_EFFECT       4 /* int (*)(int n): item n was used. Return
                                     *   non-zero to identify the item. */
#define PROPERTY_THROW_HIT_EFFECT 5 /* void (*)(int monst): item was thrown
                                     *   at a monster. */
#define PROPERTY_SCORE            6 /* Worth [score_factor]*[item value] pts */
#define PROPERTY_CARRIED_EFFECT   7
#define PROPERTY_THROWN_EFFECT    8 /* Applied immediately before firing and
                                     * then undone */
#define PROPERTY_BURN             9 /* int(*)(int n) Function for depleting fuel
                                     * Return value ignored */
//}}}

//{{{
struct itemdesc
{
	uint num;
	struct filelink name;
	struct filelink unidname;
	uchar drawchar;
	schar spot;
	uchar category;
	uint flags;
	uint weight;
	uint value;
	uint stacksize;
#ifdef SUPPORT_COLOR
	struct colorinfo color;
#endif
	struct item_properties properties;
};
//}}}
//{{{
#define ITEMFLAG_STACK         1
#define ITEMFLAG_FRAGILE       2
#define ITEMFLAG_AUTOID        4
#define ITEMFLAG_RUSTPROOF     8
#define ITEMFLAG_CURRENCY     16
#define ITEMFLAG_CHARGED      32
#define ITEMFLAG_USABLE       64
#define ITEMFLAG_HIDECHARGE  256
#define ITEMFLAG_PLUS        512
#define ITEMFLAG_GEN_CURSED 1024
#define ITEMFLAG_BREAKABLE  2048
#define ITEMFLAG_FUELED     8192
//}}}
TYPEDEF(struct itemdesc itemdesc)

TYPEDEF(int (*inventoryfilter)(const item*))

#define ITEMDESCN(a)        (*((itemdesc*)deref_file_ptr( w->itemdescs[a] )))
#define ITEMDESC(a)         (ITEMDESCN((a).type))
#define INVENTORY(a)        (w->plr.inventory[(a)])


#endif //ITEMS_H

