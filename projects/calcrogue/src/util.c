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
// util.c:
/// @file
/// @brief Miscellaneous useful functions

#include "crogue.h"

//{{{
/// \brief Appends to a string
///
/// This function is the same as 'sprintf', except that instead of replacing
/// the target string, it appends (catenates).
void catprintf(char *target, const char *format, ...)
{
	va_list varargs;
	
	while(*target) target++;
	
	va_start(varargs, format);
	vsprintf(target, format, varargs);
	va_end(varargs);
}
//}}}
//{{{
/// @brief Print into a string, and return its address.
///
/// @note Return is invalidated by the next call. If you want to still use
/// it after calling retprintf again, you must save a copy.
/// @sa vretprintf
const char *retprintf(const char *format, ...)
{
	va_list varargs;
	const char *ret;
	va_start(varargs, format);
	ret = vretprintf(format, varargs);
	va_end(varargs);
	return ret;
}
//}}}
//{{{
/// @brief Print into a string, and return its address, with a va_list.
/// @sa retprintf
const char *vretprintf(const char *format, va_list args)
{
	static char buf[128];
	vsprintf(buf, format, args);
	return buf;
}
//}}}

//{{{
/// @brief Look for a hit in a direction
///
/// Starting at (x,y), walks in direction (dir) until either:
/// @li It's gone the maximum distance
/// @li It's inside a wall
/// @li It's hit a monster
coord trace_path(int x, int y, direction dir, int distance)
{
	while(distance)
	{
		x += dir.x;
		y += dir.y;
		if( w->t[y][x].flags & TFLAG_OCCUPIED ||
		    !w->tiledescs[w->t[y][x].type].passable )
			break;
		distance--;
	}
	return COORD(x, y);
}
//}}}

//{{{
/// @brief Return a pluralized version of a string
/// 
/// Returns an appropriate string for a number of a noun. If @a num is 1,
/// the return is in the singular; otherwise, it is plural. (The rules vary
/// by language.)
/// It is an error to pass something which would pluralize to longer than
/// 63 characters.
const char *pluralize(int num, const char *str)
{
#if TARGET_LANGUAGE==ENGLISH
	int i=0, ii=0, xofyform=0;
	static char ret[64];
	char *lasttoken = ret;
	
	if(num==1) return str;
	
	while(str[i] != '\0') {
		if( str[i]==' ' && str[i+1]=='o' && str[i+2]=='f' ) {
			ret[ii++]='s';
			xofyform = 1;
		}
		if(str[i] == ' ')
			lasttoken = ret+i+1;
		ret[ii] = str[i];
		i++;
		ii++;
	}
	ret[ii]='\0';
	if( xofyform ) {
		return ret;
	}
	
	if(!strcmp(lasttoken, "tooth")) {
		lasttoken[1] = lasttoken[2] = 'e';
		return ret;
	}
	
	ret[ii++]='s';
	ret[ii]='\0';
	
	return ret;
#elif TARGET_LANGUAGE==CZECH
	static char ret[64];
	
	if(num==1) {
		return str;
	} else if(num <= 4) {
		strcpy(ret, str);
		strcat(ret, "e");
	} else {
		strcpy(ret, str);
		strcat(ret, "u");
	}
	
	return ret;
#else
	return str;
#endif
}
//}}}
//{{{
/// @brief Return the indefinite article for a string
///
/// Words beginning with a vowel get "an"; other words "a". Some special
/// cases will probably be needed in the future, but there are none yet.
/// (In particular, some words start with U but are pronounced with a Y
/// sound; these do not get "an".)
const char *indefinite_article(const char *noun)
{
#if TARGET_LANGUAGE==ENGLISH
	switch(noun[0])
	{
		case 'a': case 'A':
		case 'e': case 'E':
		case 'i': case 'I':
		case 'o': case 'O':
		case 'u': case 'U':
			return "an";
		default:
			return "a";
	}
#elif TARGET_LANGUAGE==CZECH
	return "";
#else
	/// @todo Write an #indefinite_article function for non-English languages
	return "";
#endif
}
//}}}
//{{{
/// @brief Returns the description of monster given its index.
monstdesc *find_monster_description(int m)
{
	return (monstdesc*)deref_file_ptr(w->m[m].type);
}
//}}}

//{{{
/// @brief Initialize the shuffle table
/// @sa deref_file_ptr
void init_shuffle_tab(void)
{
	int i, j, k, pos;
	uint temp;
	uint *sizes;
	
	w->shuffletranslation = (uint*)debug_malloc( sizeof(uint) * w->desc.shuffletabsize );
	
	// Make an identity table
	for(i=0; i<w->desc.shuffletabsize; i++)
		w->shuffletranslation[i] = i;
	
	// Shuffle things around
	sizes = (uint*)deref_file_ptr(w->desc.stsizes);
	pos = 0;
	for(i=0; i<w->desc.numshuffletabs; i++)
	{
		for(j=0; j<sizes[i]; j++)
		{
			k = pos+RANGE(sizes[i]-1, 0);
			
			temp = w->shuffletranslation[k];
			w->shuffletranslation[k] = w->shuffletranslation[pos+j];
			w->shuffletranslation[pos+j] = temp;
		}
		pos += sizes[i];
	}
}
//}}}

//{{{
typedef struct jumptabentry
{
	filelink target;
	uint freq;
	range condition;
} jumptabentry;
//}}}
//{{{
typedef struct jumptable
{
	uint numEntries;
	uint condition;
	jumptabentry entries[1];
} jumptable;
//}}}
static filelink deref_jump_table(jumptable *table);

ECONST filelink filelink_null = { 0, 0, 0 };

//{{{
/// @brief Dereference a file pointer.
///
/// @todo Write a decent description of how file pointers work
///
/// @sa init_shuffle_tab
/// @sa deref_file_ptr_partial
/// @sa deref_jump_table
const void *deref_file_ptr(filelink p)
{
	if(isNull(p))
		return NULL;
	else
	{
		p = deref_file_ptr_partial(p);
		
		return w->const_file[p.segment] + p.offset;
	}
}
//}}}
//{{{
/// @brief Dereference through jump tables/shuffle tables
filelink deref_file_ptr_partial(filelink p)
{
	char *target;
	
	while(p.type == PTR_JUMPTABLE || p.type == PTR_SHUFFLETAB)
	{
		target = w->const_file[p.segment] + p.offset;
		
		if(p.type == PTR_JUMPTABLE)
			p = deref_jump_table((jumptable*)target);
		else
			p = w->shuffledata[w->shuffletranslation[p.offset]];
	}
	
	return p;
}
//}}}
//{{{
static filelink deref_jump_table(jumptable *table)
{
	const jumptabentry *body = table->entries;
	uint low, high, mid, target, conditionresult;
	
	while(1)
	{
		// Weighted-pick an entry (binary search)
		low = 1;
		high = table->numEntries-1;
		target = (lrand() % body[high].freq)+1;
		
		while(1) {
			mid = (low+high)/2;
			
			if(low>=high)
				break;

			if(body[mid].freq >= target) {
				if(body[mid-1].freq < target)
					break;
				else
					high = mid-1;
			} else {
				low = mid+1;
			}
		}
		
		// Test condition
		switch(table->condition)
		{
			default:
			case jt_unconditional:
				conditionresult = 1;
				break;
			case jt_playerlevel:
				conditionresult = w->plr.level;
				break;
			case jt_dungeonlevel:
				conditionresult = MAPDESC_CURRENT.difficulty;
				break;
			case jt_avglevel:
				conditionresult = (MAPDESC_CURRENT.difficulty + w->plr.level)/2;
				break;
			case jt_class:
				conditionresult = w->plr.class;
				break;
		}
		if(conditionresult >= body[mid].condition.min && conditionresult <= body[mid].condition.max)
			break;
	}
	return body[mid].target;
}
//}}}
//{{{
int isNull(filelink p)
{
	return p.offset==0 && p.segment==0;
}
//}}}

#ifdef DEBUG_ALLOCATOR
/*
 *  4B size. Gets zeroed on free.
 *  60B write-guard. Gets set on creation, and mustn't change value.
 *  Data
 *  64B write-guard. Gets set on creation, and mustn't change value.
 */
#define GUARD_NUM 16
#define GUARD_VAL 0xDEADBEEF
#define GUARD_FREED 0xFEEBDAED
static long *alloced_blocks[32];
static int num_alloced_blocks=0;
//{{{
// Returns: 0 (all is good) or N (a block of size N is corrupt)
int debug_verify_mem(void)
{
	int ii, tmp, ret=0;
	for(ii=0; ii<num_alloced_blocks; ii++)
	{
		if((tmp=debug_verify_block(alloced_blocks[ii])))
			ret = tmp;
	}
	return ret;
}
//}}}
//{{{
int debug_verify_block(long *block)
{
	long size = *block;
	int ii;
	
	for(ii=1; ii<GUARD_NUM; ii++)
	if(block[ii] != GUARD_VAL)
	{
		panic("Clobbered negative-write guard. Block size %i, write at"
			  " offset -%i with value %li.",
		      (int)size, (int)((GUARD_NUM-ii)*sizeof(long)), block[ii]
		     );
		return size;
	}
	
	block += (size>>2) + GUARD_NUM;
	for(ii=0; ii<GUARD_NUM; ii++)
	if(block[ii] != GUARD_VAL)
	{
		panic("Clobbered positive write guard. Block size %i, write at"
		      "%i past the end.", (int)size, (int)(ii*sizeof(long))
		     );
		return size;
	}
	return 0;
}
//}}}
//{{{
void *debug_malloc(size_t S)
{
	long *ret, *pos;
	size_t size;
	int ii;
	
	// Check amount already allocated
	if(num_alloced_blocks >= 31) {
		panic("Runaway allocator!");
		return NULL;
	}
	
	// Round size up to longword
	size = S+3;
	size &= ~0x3;
	
	// Get space
	ret = malloc_throw(size + 2*GUARD_NUM*sizeof(long));
	if(ret==NULL)
		panic("Out of space!");
	
	// Write size to first long
	*ret = size;
	
	// Write guards
	for(ii=1; ii<GUARD_NUM; ii++)
		ret[ii] = GUARD_VAL;
	pos = ret+GUARD_NUM+(size>>2);
	for(ii=0; ii<GUARD_NUM; ii++)
		pos[ii] = GUARD_VAL;
	
	alloced_blocks[num_alloced_blocks++] = ret;
	
	return ret+GUARD_NUM;
}
//}}}
//{{{
void debug_free(void *ptr)
{
	long *block = ((long*)ptr) - GUARD_NUM;
	long size = *block;
	int ii;
	if(size == GUARD_FREED) {
		panic( "Multiply freed block (size %i)", (int)size );
		return;
	}
	
	// Delete from the allocated blocks list
	for(ii=0; ii<num_alloced_blocks; ii++)
	{
		if(alloced_blocks[ii] == block) {
			alloced_blocks[ii] = alloced_blocks[--num_alloced_blocks];
			break;
		}
	}
	
	debug_verify_block(block);
	*block = GUARD_FREED;
	free(block);
}
//}}}
//{{{
void *debug_calloc(size_t N, size_t S)
{
	size_t size = N*S;
	void *ret = debug_malloc(size);
	memset(ret, 0, size);
	return ret;
}
//}}}
//{{{
void *debug_realloc(void *ptr, size_t S)
{
	void *ret;
	long *blockstart, *pos;
	size_t size;
	int ii;
	if(S==0) {
		if(ptr!=NULL)
			debug_free(ptr);
		return NULL;
	}
	if(ptr==NULL)
		return debug_malloc(S);
	
	blockstart = ((long*)ptr) - GUARD_NUM;
	ret = debug_malloc(S);
	memcpy(ret, ptr, *blockstart);
	debug_free(ptr);
	return ret;
}
//}}}
#endif

