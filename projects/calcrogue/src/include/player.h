// player.h
/// @file
/// @brief Player stats, combat, movement, etc

#ifndef PLAYER_H
#define PLAYER_H

#define NUM_SKILLS 8

/// @defgroup playerstats Player Stats
///
/// Player stats. The intrinsic values of these are stored in w->plr.intrinsic,
/// and the modified values are stored in w->plr.extrinsic. The definitions
/// here are indices into either of those arrays. The first five stats -
/// #STAT_STRENGTH, #STAT_DEXTERITY, #STAT_TOUGHNESS, #STAT_MANA, and
/// #STAT_WILLPOWER - are the 'base stats'.
///
/// @sa player::intrinsic player::extrinsic
/// @{
#define NUM_STATS 22

#define STAT_STRENGTH       0
#define STAT_DEXTERITY      1
#define STAT_TOUGHNESS      2
#define STAT_MANA           3
#define STAT_WILLPOWER      4
#define STAT_DV             5
#define STAT_PV             6
#define STAT_TOHIT          7
#define STAT_DAMAGE_MIN     8
#define STAT_DAMAGE_MAX     9
#define STAT_MISSILEDAM_MIN 10
#define STAT_MISSILEDAM_MAX 11
#define STAT_MISSILE_TOHIT  12
#define STAT_LIGHTRADIUS    13
#define STAT_SPEED          14
#define STAT_REGEN          15
#define STAT_PPREGEN        16
#define STAT_MISSILES       17
#define STAT_FLAGS          18
#define STAT_FLAG_SEEINVIS      1
#define STAT_FLAG_RESIST_POISON 2
#define STAT_FLAG_CONFUSED      4
#define STAT_FLAG_HALLUCINATING 8
#define STAT_FLAG_PARALYZED     16
#define STAT_FLAG_DRAIN         32
#define STAT_FLAG_ENTANGLED     64
#define STAT_FLAG_NUMB          128
#define STAT_FLAG_TIMER         256
#define STAT_FLAG_SLIPPERY      512
#define STAT_DIGESTION      19
#define STAT_WEAPON_TYPE    20
#define STAT_COMPONENTS     21
#define STAT_COMPONENT_STAFF       1
#define STAT_COMPONENT_DAGGER      2
#define STAT_COMPONENT_SNAKETOOTH  4
#define STAT_COMPONENT_BLANKSCROLL 8
#define STAT_COMPONENT_GLOVES      16
#define STAT_COMPONENT_AM_YENDOR   32
#define STAT_COMPONENT_SHIELD      64
#define STAT_COMPONENT_FLAME       128
/// @}

#define NUM_STATMOD_TIMERS 10

/// @brief Player position and status
struct player
{
	sshort x;
	sshort y;
	
	sshort class;          ///< Player class (fighter, wizard, etc)
	
		/// Current hit points. (If zero, die)
	sshort hps;            
	
        /// @brief Max. hit points
		///
		/// Calculated from #update_player. Do not modify directly for game
		/// effects; instead, change hps_max_mod.
	sshort hps_max;
	
		/// @brief How much max HPs have been modified by.
		///
		/// Normally, HPs can be calculated solely based on Wi and To. However,
		/// hit points may be drained (by vampires, for example) or boosted.
		/// This drain/boost is represented here.
	sshort hps_max_mod;
	
		/// @brief Power points (spellcasting power)
	sshort pps;
	
		/// @brief Max. power points (spellcasting power)
		///
		/// Calculated from #update_player. Do not modify directly for game
		/// effects; instead, change hps_max_mod.
	sshort pps_max;
	
		/// @brief Modifier to max. power points
		///
		/// Power points may be "lost" mainly by casting a summoning spell.
	sshort pps_max_mod;
	
		/// @brief Fractional hit points
		///
		///Used to keep track of when the player will regenerate. See #timepass.
	ushort partialhps;
	
		/// @brief Fractional power points
		///
		///Used to keep track of when the player will regenerate. See #timepass.
	ushort partialpps;
	
		/// @brief Player level
	ushort level;
	
	schar facing_forced; /// Force-facing direction (zapping, shooting)
	schar facing; /// Last movement direction
	ushort velocity; ///< Time spent going in the same direction (used to check
	                 ///  for stumbling)
	
	struct stat_timer stat_timers[NUM_STATMOD_TIMERS];
	
		/// @brief Base stats
		/// 
		/// After modifying one of these, be sure to call #update_player so
		/// the extrinsic (modified) stats will be changed as well. Do not
		/// read these values to determine effects; look at #player::extrinsic
		/// instead.
	sshort intrinsic[NUM_STATS]; 
		/// @brief Stats after equipment modifiers
		///
		/// Calculated from #update_player based on intrinsics and worn
		/// equipment. All of the player's stats are included; to access
		/// a stat, use the corresponding index into this array. For example,
		/// strength is w->plr.extrinsic[STAT_STRENGTH].
	sshort extrinsic[NUM_STATS];
	
	sshort maximum[NUM_STATS];   ///< Highest value a stat's ever taken
	
	ushort skill[NUM_SKILLS+1];        ///< Weapon skills
	ushort skill_marks[NUM_SKILLS+1];  ///< Weapon skill marks
	
	/// Number of XP points gained last level
	/// @sa awardXP
	ulong xp;          
	
	slong satiation;   ///< Number of turns until starvation
	
	ulong debt;        ///< (Unused) money owed to a shopkeeper
	
	struct item inventory[PLRINVSIZE]; ///< Carried items
	
	slong *spellknowledge;   ///< Turns on which spells will be forgotten
	
	ulong score;       ///< The score so far
};


#endif //PLAYER_H

