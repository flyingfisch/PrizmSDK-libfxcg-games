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
// mkmap.c
/// @file
/// @brief Functions for map construction.
///
/// Note that the meta-algorithm is controlled by the data file, in
/// data/maps.m4. Also, there is another (different) algorithm in #dll/bigmap.c

#include "crogue.h"
#include "export.h"

static const schar simpledir[4][2] = {
			{0,-1}, {1,0}, {0,1}, {-1,0}	// Ordered clockwise
	};
static const schar cornerdir[4][2] = {
		{-1,-1},		{1,-1},
		
		{-1,1},			{1,1}
	};
ECONST schar dir[8][2] = {
		{-1,-1},	{0,-1},		{1,-1},
		{-1,0},					{1,0},
		{-1,1},		{0,1},		{1,1}
	};

static void connection_fill(uint x, uint y, uint s);

//{{{
void gen_map(int source, coord pos)
{
	int i;
	map_gen_method *desc;
	
	cleanup_items();
	init_items();
	
	desc = (map_gen_method*)deref_file_ptr(MAPDESC_CURRENT.genmethod);
	resize_map_store(desc->size_x, desc->size_y);
	
	w->plr.score += (MAPDESC_CURRENT.difficulty - 1) * LEVEL_SCORE_FACTOR;
	call_genericfunc(MAPDESC_CURRENT.genfunction);
	
	place_player_on_map(source, pos);
	
	for(i=0; i<MAPDESC_CURRENT.initial_population; i++)
		addrandommonst();
}
//}}}

//{{{
int mkmap_big_front(void) // Pass the call onto the DLL
{
	return mkmap_big();
}
//}}}

//{{{
void base_coat(void)
{
	map_gen_method *desc;
	int xi, yi;
	
	desc = (map_gen_method*)deref_file_ptr(MAPDESC_CURRENT.genmethod);
	
	rle_set_source(rle_pointer, desc->data);
	
	for(yi=0; yi<MAPSIZE_Y; yi++)
	for(xi=0; xi<MAPSIZE_X; xi++)
	{
		w->t[yi][xi].type = rlegetc();
		
		if(w->t[yi][xi].type == TILE_BLANK)
		{
			w->t[yi][xi].type = TILE_WALL;
			w->t[yi][xi].flags = TFLAG_MUTABLE;
		}
		else
		{
			w->t[yi][xi].flags = 0;
			if(desc->global_light)
				w->t[yi][xi].flags |= TFLAG_INTRINSIC_LIGHT;
		}
		
		w->t[yi][xi].special = 0;
	}
	
	// Clear monster table
	for(yi=0; yi<MONSTERS_MAX; yi++) {
		w->m[yi].type = filelink_null;
		w->m[yi].energy = 0;
	}
}
//}}}
//{{{
// Returns 1 (success) or 0 (failure). If it fails, the map is unchanged, but
// placement failed after too many attempts.
// Only affects areas marked as mutable, and changes area used to non-mutable.
sint place_room(room *r)
{
	int retrycount=0;
	int x, y;
	
retry:
	r->left = RANGE(MAPSIZE_X-6, 1);
	r->right = min(r->left+RANGE(8,3), MAPSIZE_X-2);
	r->top = RANGE(MAPSIZE_Y-4, 1);
	r->bottom = min(r->top+RANGE(6,3), MAPSIZE_Y-2);
	
	// Check for collisions with existing rooms
	for(y=r->top-1; y<=r->bottom+1; y++)
		for(x=r->left-1; x<=r->right+1; x++)
			if( !(w->t[y][x].flags & TFLAG_MUTABLE) )
			{
				retrycount++;
				if( retrycount>200 )
					return 0;
				else
					goto retry;
			}
	
	for(y=r->top; y<=r->bottom; y++)
		for(x=r->left; x<=r->right; x++)
		{
			w->t[y][x].flags &= ~TFLAG_MUTABLE;
			w->t[y][x].type=TILE_FLOOR;
		}
	
	return 1;
}
//}}}
//{{{
void place_hole(void)
{
	int x, y;
	
	do
	{
		x = RANGE(MAPSIZE_X-2, 1);
		y = RANGE(MAPSIZE_Y-2, 1);
	} while(!(w->t[y][x].flags & TFLAG_MUTABLE));
	
	w->t[y][x].type = TILE_FLOOR;
	w->t[y][x].flags &= ~TFLAG_MUTABLE;
}
//}}}
//{{{
void make_noise(ushort floor, ushort wall)
{
	int x, y;
	
	for(y=0; y<MAPSIZE_Y; y++)
		for(x=0; x<MAPSIZE_X; x++)
			if(w->t[y][x].flags & TFLAG_MUTABLE)
			{
				if(RANGE(100, 0) >= 45)
					w->t[y][x].type = floor;
				else
					w->t[y][x].type = wall;
			}
}
//}}}
//{{{
void cave_pass(ushort floor, ushort wall, void (*effect)(ushort floor, ushort wall, int x, int y, int neighbors))
{
	int adjcount;
	int i, ii;
	int x, y;
	
	for(y=1; y<MAPSIZE_Y-1; y++)
	for(x=1; x<MAPSIZE_X-1; x++)
	{
		if( !(w->t[y][x].flags & TFLAG_MUTABLE) )
			continue;
		
		adjcount=0;
		for(i=-1; i<=1; i++)
			for(ii=-1; ii<=1; ii++)
				if( w->t[y+i][x+ii].type != floor )
					adjcount++;
		
		effect(floor, wall, x, y, adjcount);
	}
}
//}}}
//{{{
void cb_cave_partial(ushort floor, ushort wall, int x, int y, int neighbors)
{
	if(neighbors >= nrandom(9,0))
		w->t[y][x].type = wall;
	else
		w->t[y][x].type = floor;
}
//}}}
//{{{
void cave_partial_gen(ushort floor, ushort wall)
{
	cave_pass(floor, wall, cb_cave_partial);
}
//}}}
//{{{
void cb_cave(ushort floor, ushort wall, int x, int y, int neighbors)
{
	if(neighbors >= 5)
		w->t[y][x].type = wall;
	else
		w->t[y][x].type = floor;
}
//}}}
//{{{
void cave_generation(ushort floor, ushort wall)
{
	cave_pass(floor, wall, cb_cave);
/*	int adjcount;
	int i, ii;
	int x, y;
	
	for(y=1; y<MAPSIZE_Y-1; y++)
	for(x=1; x<MAPSIZE_X-1; x++)
	{
		if( !(w->t[y][x].flags & TFLAG_MUTABLE) )
			continue;
		
		adjcount=0;
		for(i=-1; i<=1; i++)
			for(ii=-1; ii<=1; ii++)
				if( w->t[y+i][x+ii].type != floor )
					adjcount++;
		
		if(adjcount >= 5)
			w->t[y][x].type = wall;
		else
			w->t[y][x].type = floor;
	}*/
}
//}}}
static void replace(int from, int into);
//{{{
sint place_corridors(void)
{
	int num_areas=0;
	int state=0; // 0: in wall, non-0: in open area
	int x, y;
	int type, otype;
	int tries=0;
	
	// Chop map into areas for connection
	for(y=0; y<MAPSIZE_Y; y++)
	{
		for(x=0; x<MAPSIZE_X; x++)
		{
			if(state) {
				if( !TILEDESC(w->t[y][x]).passable ) {
					state=0;
				} else {
					w->t[y][x].special = num_areas;
				}
			} else {
				if( TILEDESC(w->t[y][x]).passable ) {
					state=1;
					num_areas++;
					w->t[y][x].special = num_areas;
				}
			}
		}
		state=0;
	}
	
	// Merge already-connected areas
	for(y=1; y<MAPSIZE_Y-1; y++)
		for(x=1; x<MAPSIZE_X-1; x++) {
			type = w->t[y][x].special;
			otype = w->t[y+1][x].special;
			if( type!=0 && otype!=0 && type!=otype ) {
				num_areas--;
				replace(otype, type);
			}
			otype = w->t[y][x+1].special;
			if( type!=0 && otype!=0 && type!=otype ) {
				num_areas--;
				replace(otype, type);
			}
		}
	
	// Make corridors
	do {
		// If a randomly-selected ROW connects two areas, make a tunnel
		y = RANGE(MAPSIZE_Y-2, 1);
		type = otype = 0;
		for(x=1; x<MAPSIZE_X-1; x++) {
			if( w->t[y][x].special) {
				if(!otype)
					otype = w->t[y][x].special;
				else {
					type = w->t[y][x].special;
					if(type==otype) continue;
					
					for(;;x--) {
						if(w->t[y][x].special == otype) break;
						w->t[y][x].type = TILE_FLOOR;
//						w->t[y][x].flags &= ~TFLAG_MUTABLE;
						w->t[y][x].special = type;
					}
					replace(type, otype);
					num_areas--;
					break;
				}
			}
		}
		
		// If a randomly-selected COLUMN connects two areas, make a tunnel
		x = RANGE( MAPSIZE_X-2, 1);
		type = otype = 0;
		for(y=1; y<MAPSIZE_Y-1; y++) {
			if( w->t[y][x].special ) {
				if(!otype)
					otype = w->t[y][x].special;
				else {
					type = w->t[y][x].special;
					if(type==otype) continue;
					
					for(;;y--) {
						if(w->t[y][x].special == otype) break;
						w->t[y][x].type = TILE_FLOOR;
//						w->t[y][x].flags &= ~TFLAG_MUTABLE;
						w->t[y][x].special = type;
					}
					replace(type, otype);
					num_areas--;
					break;
				}
			}
		}
		
		tries++;
		if(tries>750) {
			return 0;
		}
		
	} while(num_areas>1);
	
	for(y=0; y<MAPSIZE_Y; y++)
		for(x=0; x<MAPSIZE_X; x++)
		{
			w->t[y][x].special = 0;
		}
	
	return 1;
}

static void replace(int from, int into)
{
	int x, y;
	
	for(y=0; y<MAPSIZE_Y; y++)
	for(x=0; x<MAPSIZE_X; x++)
		if(w->t[y][x].special == from)
			w->t[y][x].special = into;
}
//}}}
//{{{
void fill_room(room *r)
{
	dll_fill_room(r);
}
//}}}
//{{{
void place_doors(void)
{
	int x, y;
	
	// A tile becomes a door if...
	// No doors on adjacent tiles
	// Two opposing EDGE tiles are walls
	// Two opposing EDGE tiles are empty space
	// At least one CORNER tile is space
	// At least one CORNER tile is a wall
	for(y=1; y<MAPSIZE_Y-1; y++)
	for(x=1; x<MAPSIZE_X-1; x++)
	{
		if( !(w->t[y][x].flags & TFLAG_MUTABLE) )
			continue;
		
		if(make_door(x, y)) {
			if(RANGE(7,0)==0)	// 1 in 8 chance a door becomes secret
				w->t[y][x].type = TILE_SDOOR;
			else
				w->t[y][x].type = TILE_CDOOR;
		}
	}
}
//}}}
//{{{
int make_door(int x, int y)
{
	int i;
	int result;
	tile *type, *otype;
	
	// A tile becomes a door if...
	// No doors on adjacent tiles
	// Two opposing EDGE tiles are walls
	// Two opposing EDGE tiles are empty space
	// At least one CORNER tile is space
	// At least one CORNER tile is a wall
	
	// Simplified version of first three rules above: simpledir rotation
	// clockwise alternates between wall and non-wall
	otype =  &w->t[y+simpledir[0][1]][x+simpledir[0][0]];
	if(TILEDESC(*otype).is_door) return 0;
	
	for(i=1; i<4; i++) {
		type = &w->t[y+simpledir[i][1]][x+simpledir[i][0]];
		if(TILEDESC(*type).is_wall == TILEDESC(*otype).is_wall || TILEDESC(*type).is_door)
			return 0;
		otype = type;
	}
	
	result=0;
	for(i=0; i<3; i++)
	{
		type = &w->t[y+cornerdir[i][1]][x+cornerdir[i][0]];
		if( TILEDESC(*type).passable )
			result|=1;
		if( TILEDESC(*type).is_wall )
			result|=2;
	}
	
	if(result==3)
		return 1;
	else
		return 0;
}
//}}}
//{{{
void place_items(void)
{
	int yi=0, xi=0;
	int ii;
	int num_items = rrandom(MAPDESC_CURRENT.num_items);
	item itm;
	
	for(ii=0; ii<num_items; ii++)
	{
		do {
			xi = nrandom(MAPSIZE_X-2, 1);
			yi = nrandom(MAPSIZE_Y-2, 1);
		} while( !w->tiledescs[ w->t[yi][xi].type ].allow_item
		         || w->t[yi][xi].flags & TFLAG_ITEM );
		
		itm = randitem(w->desc.generate_item);
		place_item(&itm, xi, yi);
	}
	
/*	for(; yi<MAPSIZE_Y; yi++)
	for(xi=0; xi<MAPSIZE_X; xi++)
	{
		if(w->tiledescs[w->t[yi][xi].type].allow_item &&
		   RANGE(35,0)==0)
		{
			itm = randitem(w->desc.generate_item);
			place_item(&itm, xi, yi);
		}
	}*/
}
//}}}
//{{{
void make_connections(void)
{
	int x, y, i;
	const spec_descriptor *specs;
	item itm;
	
	specs = MAPDESC_CURRENT.ext;
	
	// Place connections
	for( i=0; i < MAPDESC_CURRENT.numspecial; i++ )
	{
		// Random location
		if( specs[i].location.x==0 && specs[i].location.y == 0 ) {
			do {
				x = RANGE(MAPSIZE_X-2, 2);
				y = RANGE(MAPSIZE_Y-2, 2);
			} while(w->t[y][x].type != TILE_FLOOR);
		} else {
			x = specs[i].location.x;
			y = specs[i].location.y;
		}
		if( specs[i].type >= 0 )
		{
			w->t[y][x].type = specs[i].type;
			w->t[y][x].special = i;
		} else if( specs[i].type == MAPSPEC_ITEM ) {
			itm = randitem(specs[i].ext.spec_item);
			place_item(&itm, x, y);
		} else if( specs[i].type == MAPSPEC_MONSTER ) {
			addmonster(x, y, specs[i].ext.spec_monster);
		} else if( specs[i].type == MAPSPEC_CONNECT ) {
			connection_fill(x, y, i);
		}
	}
}
//}}}
//{{{
void place_player_on_map(int source, coord pos)
{
	int x, y;
	const spec_descriptor *connections;
	
	connections = MAPDESC_CURRENT.ext;
	
	// Place the player
	if( pos.x || pos.y )
	{
		w->plr.x = pos.x;
		w->plr.y = pos.y;
		w->t[w->plr.y][w->plr.x].flags |= TFLAG_OCCUPIED;
		return;
	}
	else
	{
		for(x=0; x<MAPSIZE_X; x++)
		for(y=0; y<MAPSIZE_Y; y++) {
			if( TILEDESC(w->t[y][x]).is_connection &&
				((mapdesc*)deref_file_ptr(MAPDESC_CURRENT.ext[w->t[y][x].special].ext.lvl.destmap))->mapindex == source )
			{
				w->plr.x = x;
				w->plr.y = y;
				w->t[y][x].flags |= TFLAG_OCCUPIED;
				return;
			}
		}
	}
	
	place_player_randomly();
}
//}}}

//{{{
static void connection_fill(uint x, uint y, uint s)
{
	int xi, yi;
	int left, right, top, bottom;
	w->t[y][x].special = s;
	w->t[y][x].flags |= TFLAG_TEMP;
	
	left = x-1;
	if(left<0) left=0;
	right = x+1;
	if(right>=MAPSIZE_X) right=MAPSIZE_X-1;
	top = y-1;
	if(top<0) top=0;
	bottom = y+1;
	if(bottom>=MAPSIZE_Y) bottom=MAPSIZE_Y-1;
	
	for(yi=top; yi<=bottom; yi++)
	for(xi=left; xi<=right; xi++)
	{
		if(TILEDESC(w->t[yi][xi]).is_connection &&
				!(w->t[yi][xi].flags & TFLAG_TEMP) )
			connection_fill(xi, yi, s);
	}
}
//}}}
