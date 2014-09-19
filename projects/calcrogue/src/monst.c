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
// mont.c
/// @file
/// @brief Monster AI, combat, placement etc.

#include "crogue.h"

static int monst_attack_plr(int attacknum, int monster);

// Utility
//{{{
sint monstcanmove(sshort monst, sshort x, sshort y)
{
	int target;
	
	// Standing still is always legal
	if(x==w->m[monst].x && y==w->m[monst].y)
		return 1;
	
	// Moving more than one square at a time is bad
	if(distancesquare(w->m[monst].x, w->m[monst].y, x, y) > 2)
		return 0;
	
	// Always allow attacking the player
	if(x==w->plr.x && y==w->plr.y)	
		return 1;
	
	// Only monsters that have hands can upen doors
	if(w->t[y][x].type == TILE_CDOOR
	   && MDESC(monst)->flags & MONSTFLAG_HASHANDS)
		return 1;
	
	// Some monsters can move through walls, but most can't
	if(!(w->tiledescs[w->t[y][x].type].passable))
	{
		if(!(MDESC(monst)->flags & MONSTFLAG_PASSWALL))
			return 0;
		if(x<=0 || x+1>=MAPSIZE_X || y<=0 || y+1>=MAPSIZE_Y)
			return 0;
	}
	
	// Moving onto an occupied tile is allowed if and only if one monster is
	// a pet and the other isn't
	if(w->t[y][x].flags & TFLAG_OCCUPIED)
	{
		target=monstbytile(x, y);
		if(target < 0) return 0;
		if(monst_is_pet(target) && !monst_is_peaceful(monst))
			return 1;
		else if(!monst_is_peaceful(target) && monst_is_pet(monst))
			return 1;
		else
			return 0;
	}
	return 1;
}
//}}}
//{{{
int monst_is_aligned_with_plr(int m)
{
	int dx = abs(w->m[m].x - w->plr.x);
	int dy = abs(w->m[m].y - w->plr.y);
	if(!tracevision(w->m[m].x, w->m[m].y, w->plr.x, w->plr.y))
		return 0;
	return dx==0 || dy==0 || dx==dy;
}
//}}}

//{{{
sint monstbytile(ushort x, ushort y)
{
	int i;
	
	if(!w->t[y][x].flags & TFLAG_OCCUPIED)
		return -1;
	
	for(i=0; i<MONSTERS_MAX; i++)
	{
		if(w->m[i].x==x && w->m[i].y==y && !isNull(w->m[i].type))
			return i;
	}
	return -1;
}
//}}}
//{{{
/// @brief Rearrange the monster table so that all monsters are consecutive
///
/// Rearranges the monster table to improve compressibility, by making them all
/// consecutive at the beginning of the table.
void compress_monsters(void)
{
	int i, j;
	
	for(i=j=0; i<MONSTERS_MAX; i++)
	{
		if(!isNull(w->m[i].type))
		{
			if(i != j)
			{
				memcpy(&w->m[j], &w->m[i], sizeof(monster));
				memset(&w->m[i], 0, sizeof(monster));
			}
			j++;
		}
		else
		{
			memset(&w->m[i], 0, sizeof(monster));
		}
	}
}
//}}}
//{{{
/// @brief Add a monster to the monster table and the map
///
/// If the indicated tile (@a x, @a y) is filled, the nearest clear tile is
/// used instead. If the monster table is full, the function fails silently
/// and nothing is done.
void place_monster(monster *monst, ushort x, ushort y)
{
	uint ii, xi, yi, best_x=x, best_y=y;
	sint best_distance = -1;
	static int search_start = 0;
	
	// Find an open monster handle. Rather than a full-blown linked-list
	// allocator, we'll just remember where we last got free space, and
	// start searching from there.
	ii = search_start;
	do {
		if(isNull(w->m[ii].type))
			break;
		ii++;
		if(ii >= MONSTERS_MAX) ii=0;
	} while(ii != search_start);
	
	if(ii==search_start && !isNull(w->m[ii].type))
		return;
	
	search_start = ii;
	
	// Find the nearest open tile
	for(yi=1; yi<MAPSIZE_Y-1; yi++)
	for(xi=1; xi<MAPSIZE_X-1; xi++)
	{
		if(TILEDESC(w->t[yi][xi]).passable && !(w->t[yi][xi].flags & TFLAG_OCCUPIED) && (distancesquare(xi, yi, x, y) < best_distance || best_distance == -1))
		{
			best_distance = distancesquare(xi, yi, x, y);
			best_y = yi;
			best_x = xi;
		}
	}
	x = best_x;
	y = best_y;
	
	monst->x = x;
	monst->y = y;
	w->m[ii] = *monst;
	w->t[y][x].flags |= TFLAG_OCCUPIED;
	if(w->t[y][x].flags & TFLAG_LIT)
		draw_tile(x, y);
}
//}}}

//{{{
#define NUM_HALLUNAMES 10
static const char* hallunames[NUM_HALLUNAMES] = {
	gettext("bugblatter beast of Traal"),
	gettext("greater hell beast"),
	gettext("invisible Asmodeus"),
	gettext("power grid bug"),
	gettext("pet kitten"),
	gettext("Usenet troll"),
	gettext("quantum mechanic"),
	gettext("Santa Claus"),
	gettext("Grinch"),
	gettext("giant hair")
};
//}}}
//{{{
/// Get the name of a monster from its handle
const char* monstname(sint n)
{
	if(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_HALLUCINATING)
		return hallunames[RANGE(NUM_HALLUNAMES-1, 0)];
	else
		return deref_file_ptr(MDESC(n)->name);
}
//}}}
//{{{
/// Returns whether the player can see @a m
sint player_can_see(uint m)
{
	if(!(w->t[w->m[m].y][w->m[m].x].flags & TFLAG_LIT))
		return 0;
	else if(MDESC(m)->flags & MONSTFLAG_INVISIBLE &&
	        !(w->plr.extrinsic[STAT_FLAGS] & STAT_FLAG_SEEINVIS))
		return 0;
	else
		return 1;
}
//}}}

//{{{
// This is a pre-calculation. The function is (for x=to_hit - dv)
// 28*atan(.15*x) + 50 + 0.1*x
// This function was selected experimentally, not based on any theory.
static const uint hit_chance[21] = {
		500, 542,
		583, 621,
		655, 685,
		711, 734,
		753, 770,
		785, 798,
		810, 820,
		829, 838,
		845, 852,
		859, 864,
		870
	};
//}}}
//{{{
/// @brief Determines whether an attack lands
///
/// Decides whether a hit is landed when an attacker with attack strength
/// @a to_hit attacks a defender with DV @a defense. This is similar to the
/// D20 function in that f(n, n)=1/2 and df/dH = df/dDV. However, the scale
/// is extended to +/- 20 instead of +/- 10, and the first derivitave falls
/// off as the to-hit and defense values separate.
/// (Maybe some graphs should go here?)
int calculate_hit(int to_hit, int defense)
{
	uint chance;
	sint dv_difference = to_hit - defense;
	if(dv_difference>20) dv_difference = 20;
	if(dv_difference<-20) dv_difference = -20;
	if(dv_difference>=0) chance = hit_chance[dv_difference];
	else                 chance = 1000-hit_chance[-dv_difference];
	
	if(lrand()%1000 <= chance)
		return 1;
	else 
		return 0;

}
//}}}

//{{{
sint monst_facing(sint monst)
{
	int df = delta_facing(player_facing(),
	                      facing_towards(w->m[monst].x, w->m[monst].y));
	if(df > 2)
		return FACING_BEHIND;
	else if(df == 2)
		return FACING_FLANK;
	else
		return FACING_FRONT;
}
//}}}

//{{{
int monst_hit_plr(int attacknum, int monster)
{
	return calculate_hit(
		MATTACK(monster, attacknum).tohit +
		(monst_facing(monster) == FACING_BEHIND)*3 +
		2*MDESC(monster)->level,
		w->plr.extrinsic[STAT_DV]);
}
//}}}
//{{{
int monst_hit_monst(int attacknum, int attacker, int target)
{
	return calculate_hit(
		MATTACK(attacker, attacknum).tohit +
		2*MDESC(attacker)->level + 2,
		MDESC(target)->dv);
}
//}}}
//{{{
static int monst_attack_plr(int attacknum, int monster)
{
	int damage;
	monstattack *attacks;
	
	attacks = MDESC(monster)->attacks;
	damage = rrandom( attacks[attacknum].damage );
	
#ifdef DEBUG_BALANCE
	if(w->debug_mode)
		message("(%i,%i)v[%i,%i]", MATTACK(monster, attacknum).tohit +
			2*MDESC(monster)->level, damage,
			w->plr.extrinsic[STAT_DV], w->plr.extrinsic[STAT_PV]);
#endif
	return call_attackfunc( attacks[attacknum].type, monster, damage, -1 );
}
//}}}

//{{{
int monst_attack_type(int trigger, int monst, int always_hit)
{
	int i;
	int ret=0;
	
	for(i=0; i<MDESC(monst)->numattacks; i++)
	{
		if(MATTACK(monst, i).trigger == trigger)
		{
			if(always_hit || monst_hit_plr(i, monst)) {
				if(monst_attack_plr(i, monst))
					ret=1;
			} else {
				ret = 1;
#ifdef DEBUG_BALANCE
				if(w->debug_mode)
					message("(%i,-)v[%i,%i]",
						MATTACK(monst,i).tohit + 2*MDESC(monst)->level,
						w->plr.extrinsic[STAT_DV], w->plr.extrinsic[STAT_PV]);
#endif
				if(monst_facing(monst) == FACING_BEHIND)
					message(gettext("The %s attacks from behind but misses!"),
						monstname(monst));
				else
					message(gettext("The %s misses!"), monstname(monst));
			}
		}
		
		if(isNull(w->m[monst].type))
			return ret;
	}
	return ret;
}
//}}}

//{{{
void addrandommonst(void)
{
	int x, y;
	
	// Find an open tile
	do
	{
		x = RANGE( MAPSIZE_X-1, 1);
		y = RANGE( MAPSIZE_Y-1, 1);
	}
	while( !TILEDESC(w->t[y][x]).passable || TILEDESC(w->t[y][x]).is_connection
	       || (w->t[y][x].flags & TFLAG_OCCUPIED) );
	addmonster(x, y, MAPDESC_CURRENT.monstertab);
}
//}}}
//{{{
void addmonster(ushort x, ushort y, filelink type)
{
	uint i;
	const filelink desc = deref_file_ptr_partial(type);
	int num_place = 1;
	monster monst;
	const monstdesc *ptr_desc = (const monstdesc*)deref_file_ptr(desc);
	
	if(ptr_desc->flags & MONSTFLAG_GROUP)
		num_place = 4;
	
	for(i=0; i<num_place; i++)
	{
		monst.type = desc;
		monst.hps = ptr_desc->hps_max;
		monst.energy = 0;
		monst.power = ptr_desc->power_max;
		monst.flags = ptr_desc->ai;
		place_monster(&monst, x, y);
	}
}
//}}}
//{{{
// For data file use (struct by value is too awkward)
void addmonster_ptr(filelink *type, ushort x, ushort y)
{
	addmonster(x, y, *type);
}
//}}}
//{{{
void monst_remove(uint monstid)
{
	w->t[w->m[monstid].y][w->m[monstid].x].flags &= ~TFLAG_OCCUPIED;
	w->m[monstid].type = filelink_null;
	draw_tile(w->m[monstid].x, w->m[monstid].y);
}
//}}}
//{{{
void monst_takedamage(sint monstid, sint damage, sint is_melee)
{
	if(damage<0)
		return;
	
	damage = max( damage-MDESC(monstid)->pv, damage/4 );
	w->m[monstid].hps -= damage;
	
	if( w->m[monstid].hps <= 0 )
	{
		if(player_can_see(monstid))
			message(gettext("The %s is killed!"), monstname(monstid));
		awardXP(MDESC(monstid)->xp);
		
		if(is_melee)
		{
			// Look for death-defense attacks
			monst_attack_type(MTRIGGER_DEATH, monstid, 0);
		}
		
		call_killfunc(MDESC(monstid)->deathfunc, monstid);
		monst_remove(monstid);
		return;
	} else {
		if(is_melee)
		{
			// Look for defensive attacks
			monst_attack_type(MTRIGGER_DEFENSE, monstid, 0);
		}
	}
	if(w->m[monstid].hps*4 <= MDESC(monstid)->hps_max && RANGE(1,0) &&
	   !(MDESC(monstid)->flags))
		scare_monster(monstid);
}
//}}}
//{{{
void monst_heal(uint monstid, uint amt)
{
	w->m[monstid].hps += amt;
	
	if(w->m[monstid].hps > MDESC(monstid)->hps_max)
		w->m[monstid].hps = MDESC(monstid)->hps_max;
}
//}}}

// AI
//{{{
void monstmoveto(sint monst, sint x, sint y)
{
	int target;
	if(w->t[y][x].flags & TFLAG_OCCUPIED)
	{
		if(x == w->plr.x && y == w->plr.y)
		{
			if(!monst_attack_type(MTRIGGER_NORMAL, monst, 0))
				monst_attack_type(MTRIGGER_RANGED, monst, 1);
		}
		else
		{
			target = monstbytile(x,y);
			if(target >= 0 && target != monst)
				monst_swing_monst(monst, target, MTRIGGER_NORMAL);
		}
	} else if(w->t[y][x].type == TILE_CDOOR)
	{
		w->t[y][x].type = TILE_ODOOR;
		draw_tile(x, y);
		calc_light();
	} else {
		w->t[w->m[monst].y][w->m[monst].x].flags &= ~TFLAG_OCCUPIED;
		draw_tile(w->m[monst].x, w->m[monst].y);
		w->m[monst].y = y;
		w->m[monst].x = x;
		w->t[y][x].flags |= TFLAG_OCCUPIED;
		draw_tile(x, y);
	}
}
//}}}
//{{{
ushort monst_detect_player(ushort i)
{
	int d = distancesquare(w->m[i].x, w->m[i].y, w->plr.x, w->plr.y);
	
	if(d > 144)
		return 0;
	if( d<=16 ) // Can hear the player
		return 1;
	else if( (w->t[w->m[i].y][w->m[i].x].flags & TFLAG_LIT) )
        // Reasonably near the player (12 tiles) and can see the player.
		// FIXME: This limits vision to player's sight range
		return 1;
	else
		return 0;
}
//}}}
//{{{
ushort monst_can_attack_player(ushort i)
{
	return distancesquare(w->m[i].x, w->m[i].y, w->plr.x, w->plr.y) <= 2;
}
//}}}

//{{{
void monstmoverandomly(sint i, uint attack)
{
	int x, y;
	do
	{
		x = w->m[i].x + RANGE(2, 0) - 1;
		y = w->m[i].y + RANGE(2, 0) - 1;
	} while( !monstcanmove(i, x, y) ||
	        (!attack && x==w->plr.x && y==w->plr.y) );
	monstmoveto(i, x, y);
}
//}}}
//{{{
void monstmovetowardsplayer(sint i)
{
	uint best_val = 0xFFFF;
	sint best_x = w->m[i].x, best_y = w->m[i].y;
	sint xi, yi;
	
	for(yi = w->m[i].y-1; yi <= w->m[i].y+1; yi++)
		for(xi = w->m[i].x-1; xi <= w->m[i].x+1; xi++)
		{
			if( monstcanmove( i, xi, yi ) &&  // Move is legal
				distancesquare(xi, yi, w->plr.x, w->plr.y) < best_val ) // And is an improvement
			{
				best_val = distancesquare(xi, yi, w->plr.x, w->plr.y);
				best_x = xi;
				best_y = yi;
			}
		}
	
	monstmoveto(i, best_x, best_y);
}
//}}}
//{{{
void monstmoveawayfromplayer(sint i)
{
	uint best_val = 0;
	sint best_x = w->m[i].x, best_y = w->m[i].y;
	sint xi, yi;
	
	for(yi = w->m[i].y-1; yi <= w->m[i].y+1; yi++)
		for(xi = w->m[i].x-1; xi <= w->m[i].x+1; xi++)
		{
			if( monstcanmove( i, xi, yi ) &&  // Move is legal
				distancesquare(xi, yi, w->plr.x, w->plr.y) > best_val ) // And is an improvement
			{
				best_val = distancesquare(xi, yi, w->plr.x, w->plr.y);
				best_x = xi;
				best_y = yi;
			}
		}
	
	monstmoveto(i, best_x, best_y);
}
//}}}
//{{{
int monst_is_peaceful(int monstid)
{
	if(w->m[monstid].flags & (MONST_PET | MONST_FRIENDLY))
		return 1;
	else
		return 0;
}
//}}}
//{{{
int monst_is_pet(int monstid)
{
	if(w->m[monstid].flags & MONST_PET)
	     return 1;
	else return 0;
}
//}}}

//{{{
void monst_anger(ushort monstid)
{
	int flags = w->m[monstid].flags;
	if(flags & MONST_CUST_ANGER) {
		call_aifunc( MDESC(monstid)->ai_anger, monstid );
	} else {
		w->m[monstid].flags |= MONST_ACTIVE;
		w->m[monstid].flags &= ~(MONST_PEACEFUL|MONST_FRIENDLY|MONST_SHOPKEEP|MONST_CUST_WHEN_ACTIVE);
		if(flags & MONST_FRIENDLY) {
			message(gettext("The %s gets angry!"), monstname(monstid));
		}
	}
}
//}}}
//{{{
void scare_monster(ushort monstid)
{
	if(MDESC(monstid)->flags & MONSTFLAG_FEARLESS)
		return;
	scare_monster_force(monstid);
}
//}}}
//{{{
void scare_monster_force(int monstid)
{
	if(player_can_see(monstid))
		message(gettext("The %s turns to flee!"), monstname(monstid));
	w->m[monstid].flags |= MONST_SCARED;
}
//}}}
//{{{
void confuse_monster(ushort monstid)
{
	if(player_can_see(monstid))
		message(gettext("The %s is dazed."), monstname(monstid));
	w->m[monstid].flags |= MONST_CONFUSED;
}
//}}}

//{{{
void gather_wandering_monsters(void)
{
	int ii;
	for(ii=0; ii<MONSTERS_MAX; ii++)
	{
		if(isNull(w->m[ii].type)) continue;
		if(monst_is_pet(ii) ||
			(monstcanmove(ii, w->plr.x, w->plr.y) && !monst_is_peaceful(ii)) )
		{
			w->wandering_monsters = debug_realloc(w->wandering_monsters,
			    (++w->wandering_monsters_num)*sizeof(monster));
			w->wandering_monsters[w->wandering_monsters_num-1] = w->m[ii];
			monst_remove(ii);
		}
	}
}
//}}}
//{{{
void place_wandering_monsters(void)
{
	int ii;
	if(w->wandering_monsters)
	{
		for(ii=0; ii<w->wandering_monsters_num; ii++)
			place_monster(&w->wandering_monsters[ii], w->plr.x, w->plr.y);
		debug_free(w->wandering_monsters);
		w->wandering_monsters = NULL;
		w->wandering_monsters_num = 0;
	}
}
//}}}

