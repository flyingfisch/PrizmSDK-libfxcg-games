// struct_world.h
/// @file
/// @brief Global game state

#ifndef STRUCT_WORLD_H
#define STRUCT_WORLD_H

/// @brief A tile on the map
///
/// The size of this struct is very important, since there could potentially
/// be as many as 2^14 instances at a time (128x128 map).
///
/// The 'special' field is either an index into the #mapdesc's extensions,
/// or a tile-type-specific option. It may be removed at some point in the
/// future, as the information it is used for is moved into a different
/// place and format.
struct tile
{
	uchar type;
	uchar flags;
	ushort special;
};
TYPEDEF(struct tile tile)

/// @defgroup tileflags Tile flags
/// Possible values for #tile::flags. These can be combined either with
/// + (plus) or | (bitwise or).
/// @{
	/// Tile has been seen at some point
#define TFLAG_EXPLORED        1
	/// Tile should be represented as floor, even though it's something else
#define TFLAG_HIDETRAP        2
	/// Tile has something standing on it (player or monster)
#define TFLAG_OCCUPIED        4
	/// Tile is within the range of the player's light and vision
#define TFLAG_LIT             8
	/// Tile is lit when in view even if the player has no light
#define TFLAG_INTRINSIC_LIGHT 16
	/// (During map generation) Tile may be changed by the map generator
#define TFLAG_MUTABLE         32
	/// (During map generation) Temporary flag which may be used for anything
#define TFLAG_TEMP            64
	/// Tile has an item on it
#define TFLAG_ITEM            128
/// @}

#define NUM_FOUNTAIN_TYPES      7

#ifdef DATA_FILE
#	define MAPSIZE_X   (get_w()->mapsize_x)
#	define MAPSIZE_Y   (get_w()->mapsize_y)
#else
#	define MAPSIZE_X   (w->mapsize_x)
#	define MAPSIZE_Y   (w->mapsize_y)
#endif

/// @brief The current game state
///
/// Everything about the current game state is bundled up into this structure
/// (or pointers contained within it), for easy saving and easy access. There
/// is exactly one instance, which is always pointed to by #w (and which can
/// be retrieved with #get_w). Being in a single structure means that a single
/// pointer, which is stored in a register, can be used to keep track of it,
/// and this structure can be written verbatim to save the game (BUT pointers
/// will need to be re-set when restoring.)
/// @sa w get_w
struct world
{	
	struct world_descriptor desc;
	
	ulong num_segments;   ///< Number of segments in the constant data file
	uchar **const_file;   ///< Segment contents in the constant data file
	
		/// DLL export table (export from DLL to main program and data file)
	CONST void** dll_functions;
#ifndef DATA_FILE
		/// DLL interface table (export from main program to DLL)
	const struct export_functions *dll_interface;
#else
	void *dll_interface;
#endif
	
		/// Pointer to table of tile descriptors which specifies what the tiles
		/// look like and qualities such as whether they are transparent to
		/// vision
	struct tiledesc*  tiledescs;
	struct filelink current_map_link;
	CONST struct mapdesc*   current_map;
	CONST struct filelink*  itemdescs;
	CONST struct spelldesc* spelldescs;
	CONST struct shopdesc*  shopdescs;
	CONST struct classdesc* playerclasses;
	CONST struct filelink*  shuffledata;
	uint* shuffletranslation;
	
	ushort mapsize_x; ///< Size of the current level
	ushort mapsize_y; ///< Size of the current level
	struct tile **t;  ///< Layout of the current level
	
		/// Items on the floor of the current level
	struct itemsinfo items;
	
		/// Monsters occupying the current level
	struct monster m[MONSTERS_MAX];
	struct player plr;
	
		/// @brief Monsters currently between levels
		///
		/// Wandering monsters: Monsters not on a level, but between levels
		/// (usually because they're following the player through stairs, but
		/// they might stay here for awhile if the player enters a
		/// no-monsters-allowed level.
	struct monster *wandering_monsters;
		/// Number of wandering monsters
	ushort wandering_monsters_num;
	
	ushort pad;
	
		/// @brief Which item types have been identified
		///
		/// One BIT per item type, indicating whether or not it's been
		/// identified. Access only through #identify_type and
		/// #type_is_identified
	ulong *itemids;
		/// Number of player-turns into the game
	ulong time;         
		/// The level number (index in mapdescs)
	uchar level;        
		/// The highest level number the player has ever been on
	uchar maxlevel;   
		/// Whether there should be a message displayed in the message field (to
		/// decide when to clear old messages).
	uchar messagevis;	
		/// @brief Whether the player is interrupted
		/// 
		/// To handle a multi-turn action, set this to zero, and continue
		/// unless/until it becomes nonzero. Any #message will set this.
	uchar interrupt;	
	
		/// Extra space reserved for use by data-file-defined functions
	uchar game_flags[64];
	
//	uchar options[NUM_OPTIONS];
	
#ifdef ALLOWDEBUG
		/// Whether in debug mode or not
	uchar debug_mode;
#endif
};
TYPEDEF(struct world world)

#endif
