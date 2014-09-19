// struct_constdata.h
/// @file
/// @brief Miscellaneous structures in the data file
///
/// This is a bilingual header file.

#ifndef CONSTDATA_H
#define CONSTDATA_H

//{{{
/// @brief Data-file header
struct world_descriptor
{
	/// Insertion points into data-file functions
	struct filelink function_entries;
	
	/// @brief Position in the data file of the tiles table
	///
	/// @sa tiledesc
	struct filelink tileoffset;
	
	/// @brief Item descriptions table
	///
	/// Each item is kept track of with an integer, which corresponds to
	/// its position in the item types array. This points to that array.
	/// When the data file is loaded, this pointer is dereferenced and
	/// stored in #w->itemdescs for easier use.
	/// (To get the description of an item, do not use this directly; instead,
	/// use the #ITEMDESC or #ITEMDESCN macro.)
	/// @sa itemdesc
	/// @sa world::itemdescs
	struct filelink itemoffset;
	
	/// @brief Item generation table
	///
	/// Whenever an item is to be generated, the game will dereference this
	/// pointer. Since pointers in the data file may correspond to probability
	/// tables, this allows random item selection.
	/// @sa itemdesc
	struct filelink generate_item;
	
	/// @brief Pointer to 'gold piece' item description
	///
	/// For handling shopkeepers, the game needs to know where the 'gold piece'
	/// item description is; this points to it.
	/// @sa itemdesc
	struct filelink generate_currency;
	
	/// @brief Player's starting inventory
	///
	/// Points to an int (number of items) followed by an array of #item.
	/// See #data/items.m4. Note that this does depend on what class the
	/// player is, but that is handled entirely by the dereferencing
	/// operation.
	/// @sa item
	struct filelink startinginventory;
	
	/// @brief List of spells
	///
	/// Points to the spells table, an array of #spelldesc. This is
	/// dereferenced and cached as #w->spelldescs.
	/// @sa world::spelldescs @sa spelldesc
	struct filelink spelllist;
	
	/// @brief Table of player classes
	///
	/// Points to the list of player classes, an array of #classdesc. On load,
	/// this is dereferenced and cached as #w->playerclasses; there is no need
	/// to use it directly.
	/// @sa world::playerclasses
	/// @sa classdesc
	struct filelink playerclasses;
	struct filelink stsizes;
	struct filelink stdata;
	struct filelink shopdescs;
	struct filelink titlescreen_light_plane;
	struct filelink titlescreen_dark_plane;
	
	uint itementries;       ///< Number of item types
	uint numspells;         ///< Number of spells
	uint numplayerclasses;  ///< Number of player classes
	uint tutorialclass;     ///< Index of the 'tutorial' player class
	uint numshuffletabs;    ///< Number of shuffle tables
	uint shuffletabsize;    ///< Number of shuffle table entries
	uint numshoptypes;      ///< Number of types of shops
	uint padding;
	
	struct filelink mainmap;
	struct filelink tutorialmap;
	
	struct filelink misc_links;
};
//}}}

//{{{
/// Data for a type of tile
struct tiledesc
{
	uchar transparent;    ///< Whether it can be seen through
	uchar drawchar;       ///< How it should be drawn on the map
	uchar passable;       ///< Whether it can be walked on
	uchar is_connection;  ///< Whether it leads to another map
	uchar allow_item;     ///< Whether items can be generated here
	uchar is_wall;        ///< Whether it is a wall
	uchar is_door;        ///< Whether it is a door
	uchar padding;
#ifdef SUPPORT_COLOR
	struct colorinfo color;
	short pad2;
#else
	long pad2;
#endif
	struct filelink step_function;  ///< Function to call when stepped on
};
//}}}
//{{{
/// Data for a type of spell
struct spelldesc
{
	struct filelink name;        ///< The spell's (displayed) name
	uint num;                    ///< The spell's index
	uint pp_cost;                ///< Mana cost of the spell
	uint difficulty;             ///< Difficulty of learning the spell
	struct filelink learn_func;  ///< Function called when spell is learned
	struct filelink function;    ///< Function called when spell is cast
};
//}}}
#define NUM_BASE_STATS 5
#define SPELLTYPE_ARCANE     0
#define SPELLTYPE_DIVINE     1
#define SPELLTYPE_NECROMANCY 2
//{{{
/// \brief Information describing a player class (eg fighter, wizard).
///
/// Instances of this structure appear (sort of) in data/playerclass.m4.
/// The layout of this structure must remained synchronized with the
/// corresponding macro in that file.
struct classdesc
{
	struct filelink name; ///< Link to string: name of the class
	
	uint base_stat[NUM_BASE_STATS]; ///< Minimum attributes (St, Dx, To, Wi, Mn)
	uint max_stat[NUM_BASE_STATS];  ///< Maximum starting attributes
	uint free_points;           ///< \brief Number of points to assign randomly
	                            ///  to attributes
	uint spellcasting[3];       ///< \brief Spellcasting skill for each type of
	                            ///  spell (#SPELLTYPE_ARCANE,
	                            ///  #SPELLTYPE_DIVINE, #SPELLTYPE_NECROMANCY).
	uint weapon_skill_factor;   ///< \brief Multiplier to number of marks needed
	                            ///  to gain a skill level.
	uint weapon_skill_start;    ///< \brief Starting weapon skill (all weapon
	                            ///  types)
};
//}}}
//{{{
/// Data describing a type of shop
struct shopdesc
{
	struct filelink name;            ///< The name of the shop
	struct filelink shopkeeper;      ///< The creature inside
	struct filelink generate_item;   ///< Table of generated items
};
//}}}

#endif //CONSTDATA_H

