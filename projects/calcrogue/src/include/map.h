// map.h
/// @file
/// @brief Map construction

#ifndef MAP_H
#define MAP_H

//{{{
struct spec_descriptor_ext_level
{
	struct filelink destmap;
	struct coord destcoord;
};
TYPEDEF(struct spec_descriptor_ext_level spec_descriptor_ext_level)
//}}}
//{{{
union spec_descriptor_ext
{
	struct spec_descriptor_ext_level lvl;
	struct filelink spec_item;
	struct filelink spec_monster;
	uint sign_id;
};
TYPEDEF(union spec_descriptor_ext spec_descriptor_ext)
//}}}
//{{{
struct spec_descriptor
{
	sint type; // If >=0, this is a tile index; else, corresponds to defs below
	struct coord location;
	union spec_descriptor_ext ext;
};
TYPEDEF(struct spec_descriptor spec_descriptor)
//}}}
//{{{
#define MAPSPEC_ITEM    -1
#define MAPSPEC_MONSTER -2
#define MAPSPEC_CONNECT -3
#define MAPSPEC_KEY     -4 /* Key which opens all locked doors on this level */
#define MAPSPEC_USER1   -5 /* For area-specific *connection* specs. */
#define MAPSPEC_USER2   -6 /* Use with funky level generators. */
//}}}

//{{{
/// @brief Description of a level in the game world
///
/// This structure appears in the constant data file in data/maps.m4 and
/// data/maps.m4h. The definition given here must remain synchronized with
/// the one there.
/// @sa world::current_map world::current_map_link world::level
struct mapdesc
{
		/// (Ref to string) Map name
	struct filelink name;
		/// Evaluates to #monstdesc*, to generate a monster.
	struct filelink monstertab;
		///< Evaluates to #map_gen_method
	struct filelink genmethod;
		///< Function to call when map is created
	struct filelink genfunction;
		///< Function to call when map is entered
	struct filelink enterfunction;
	
	uint mapindex;     ///< Map's index into the maps table
	
	/// @brief Special properties of this level
	///
	/// Possible values are in @ref mapflags "Map Flags".
	uint flags;
	
	/// @brief How difficult this map is. Affects jump tables (thus, monster
	/// types) and the number of points awarded for reaching it.
	uint difficulty;   
	
	struct range num_items;  ///< Number of items on this map
	uint initial_population; ///< Number of monsters on the map
	uint numspecial;         ///< Number of additional properties this map has
	struct spec_descriptor ext[1];
};
TYPEDEF(struct mapdesc mapdesc)
//}}}
 //{{{ Map flags
/// @defgroup mapflags Map Flags
///
/// Special properties maps may take. These may be combined using either
/// + (plus) or | (bitwise or).
/// @sa mapdesc::flags
/// @{
	/// No monsters are generated on this level
#define MAPFLAG_NORANDMONST 1
	/// This is an outdoor area
#define MAPFLAG_OUTSIDE     4
	/// Monsters may not this level (no pets)
#define MAPFLAG_NOMONST     8
	/// Teleporting doesn't work here
#define MAPFLAG_NOTELEPORT  16
	/// The walls here can be dug in easily
#define MAPFLAG_EASYDIGGING 32
/// @}
//}}}
//{{{
/// A base coat for map generation
struct map_gen_method
{
	ushort size_x;       ///< Map size (x)
	ushort size_y;       ///< Map size (y)
	uchar global_light;  ///< Indicates the map should be fully lit
	uchar padding;
	char data[1];        ///< Tiles placed before beginning map generation
};
TYPEDEF(struct map_gen_method map_gen_method)
//}}}

//{{{
/// A room on a map being generated
struct room
{
	short left;
	short right;
	short top;
	short bottom;
};
TYPEDEF(struct room room)
//}}}

#endif //MAP_H

