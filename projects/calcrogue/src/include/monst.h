// monst.h
/// @file
/// @brief Monster movement, combat, etc

#ifndef MONST_H
#define MONST_H

//{{{
/// @brief A monster's attack
///
/// Actually, more like 'an action a monster can do'. Other actions like
/// summoning help, shooting missiles, breathing fire, etc., are handled
/// with this as well.
struct monstattack
{
		/// @brief (Interpretted) function called when this attack is made.
		///
		/// See data/monsters.m4 for examples.
	struct filelink type;
		/// How much damage is dealt
	struct range damage;
		/// Modifier to chance-to-hit
	schar tohit;
		/// @brief When this attack is made
		///
		/// May be one of: #MTRIGGER_NORMAL, #MTRIGGER_RANGED,
		/// #MTRIGGER_DEFENSE, #MTRIGGER_DEATH, #MTRIGGER_SEEN,
		/// #MTRIGGER_PREEMPT
	uchar trigger;
};
TYPEDEF(struct monstattack monstattack)

	/// monstattack::trigger     When the monster is in melee range
#define MTRIGGER_NORMAL       0
	/// monstattack::trigger     When the monster's lined up for a ranged attack
#define MTRIGGER_RANGED       1
	/// monstattack::trigger     After the monster is hit (but not killed)
#define MTRIGGER_DEFENSE      2
	/// monstattack::trigger     After the monster is killed
#define MTRIGGER_DEATH        3
	/// monstattack::trigger     When the monster sees the player and has
	///                          nothing more specific to do
#define MTRIGGER_SEEN         4
	/// monstattack::trigger     Before an attack is made (prevents the attack
	///                          from registering).
#define MTRIGGER_PREEMPT      5
//}}}
//{{{
/// @brief Description of a type of monster
///
/// Data file only. See #data/monsters.m4. This structure definition must
/// remain synchronized with the data there.
struct monstdesc
{
	struct filelink name;    ///< Points to monster's name
	uchar drawchar;   ///< Symbol used to represent monster on map
	uchar level;      ///< Monster's level (affects its chance to hit)
	uchar dv;         ///< Monster's defense value
	uchar pv;         ///< Monster's protection value
	sint hps_max;     ///< Monster's max. hit points
	uchar speed;      ///< Monster speed (normal speed is 10)
	uchar power_max;  ///< Monster's max power.
	
	/// Starting AI state
	///
	/// @sa monster::flags aiflags
	ushort ai;
	
	struct filelink ai_active;  ///< Function to use when active (if any)
	struct filelink ai_idle;    ///< Function to use when idle (if any)
	struct filelink ai_anger;   ///< Function to use when angered (if any)
	struct filelink deathfunc;  ///< Function to use when killed (if any)
	struct filelink chatfunc;   ///< Function to use when chatted with (if any)
	
	uchar numattacks;  ///< Number of attacks in \a attacks
	
	/// XP given for a kill
	/// @sa awardXP
	uint xp;           
	
	/// @brief Special monster abilities/properties
	///
	/// May be any combination of the abilities in
	/// \ref monstflags "Monster Flags"
	/// #sa monstflags
	uint flags;
	
#ifdef SUPPORT_COLOR
	struct colorinfo color;   ///< (PC only) Color used for this monster on the map
#endif
	
	struct monstattack attacks[1]; ///< Attack details
};
TYPEDEF(struct monstdesc monstdesc)
//}}}

//{{{
/// @brief A monster
///
/// Instances are kept in w->m
/// @sa w::m MONSTERS_MAX
struct monster
{
	struct filelink type; ///< Link to corresponding #monstdesc, or null if dead
	sshort hps;
	uchar energy;  ///< Moves the monster can make (move when >player speed)
	uchar power;   ///< Mana. Regenerates at 1 point per turn.
	sshort x;
	sshort y;
		/// @brief Controls monster behavior.
		///
		/// Flags may be any combination of @ref aiflags "AI Flags". When the
		/// monster is created, this value is copied from its description
		/// #monstdesc::ai.
		/// @sa aiflags monstdesc monstdesc::ai
	ushort flags;  
	
	ushort pad2; ///< Unused
};
//}}}
 //{{{ Monster AI flags
/// @defgroup aiflags Monster AI flags
/// Possible values for #monster::flags. These can be combined either with
/// + (plus) or | (bitwise or).
/// @{
	/// Monster is peaceful
#define MONST_PEACEFUL          1
	/// Monster is a pet
#define MONST_PET               2
	/// Monster is friendly
#define MONST_FRIENDLY          4
	/// Monster is active (senses player)
#define MONST_ACTIVE            8
	/// Use a custom routine when active
	///
	/// The routine used is #monstdesc::ai_active.
	/// @sa monstdesc::ai_active
#define MONST_CUST_WHEN_ACTIVE  16
	/// Use a custom routine when idle
	///
	/// The routine used is #monstdesc::ai_idle.
	/// @sa monstdesc::ai_idle
#define MONST_CUST_WHEN_IDLE    32
	/// Use a custom function when angered
	///
	/// The routine used is #monstdesc::ai_anger. It is called once when
	/// the monster becomes angry, and is not called again after that.
	/// @sa monstdesc::ai_anger
#define MONST_CUST_ANGER        64
	/// Wander around when idle
#define MONST_WANDER_WHEN_IDLE  128
	/// Confused
#define MONST_CONFUSED          256
	/// Frightened
#define MONST_SCARED            512
	/// Is a mimic
#define MONST_MIMIC             1024
	/// Is a shopkeeper
#define MONST_SHOPKEEP          2048
	/// Is paralyzed
#define MONST_PARALYZED         4096

#define AI_BRAINDEAD    (MONST_PEACEFUL)
#define AI_HOSTILE      (MONST_WANDER_WHEN_IDLE)
#define AI_CHASE        (AI_HOSTILE|MONST_ACTIVE)
#define AI_SCARED       (AI_CHASE|MONST_SCARED)
#define AI_CONFUSED     (AI_CHASE|MONST_CONFUSED)
#define AI_DONTCARE     (MONST_PEACEFUL|MONST_WANDER_WHEN_IDLE)
#define AI_PEACEFUL     (MONST_FRIENDLY|MONST_WANDER_WHEN_IDLE)
#define AI_SHOPKEEP     (MONST_FRIENDLY|MONST_SHOPKEEP|MONST_CUST_ANGER)
#define AI_PET          (MONST_PET|MONST_ACTIVE)
#define AI_MIMIC        (MONST_CUST_WHEN_IDLE|MONST_MIMIC)
#define AI_WEBSPINNER   (MONST_CUST_WHEN_IDLE|MONST_CUST_WHEN_ACTIVE)
#define AI_NPC_SIT      (MONST_FRIENDLY|MONST_CUST_WHEN_ACTIVE)
/// @}
//}}}
 //{{{ Monster flags
/// @defgroup monstflags Monster Flags
///
/// Special monster abilities/properties, specified with #monstdesc::flags.
/// These may be combined using either + (plus) or | (bitwise or).
/// @sa monster::flags
/// @{
	/// Regenerates quickly
#define MONSTFLAG_REGENERATE   1
	/// Is invisible
#define MONSTFLAG_INVISIBLE    2
	/// Cannot be frightened
#define MONSTFLAG_FEARLESS     4
	/// Walks through walls
#define MONSTFLAG_PASSWALL     8
	/// Appears in groups of 4
#define MONSTFLAG_GROUP        16
	/// Can manipulate doors
#define MONSTFLAG_HASHANDS     32
	/// Is repelled by crosses
#define MONSTFLAG_VAMPIRIC     64
/// @}
//}}}
//{{{
#define FACING_FRONT 0
#define FACING_FLANK 1
#define FACING_BEHIND 2
//}}}
TYPEDEF(struct monster monster)

#endif //MONST_H

