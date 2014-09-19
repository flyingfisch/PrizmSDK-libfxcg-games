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
// interpret_aux.c
/// @file
/// @brief Helper functions and function table for calls from the data file
/// into the main program.

#include "crogue.h"

/// \brief Export of a function from the main program to the data file
///
/// This is used to export access to functions from the main program to the
/// data file. Since the data file is written in an interpretted language,
/// the calls must be translated. helper_func is used to do the translation;
/// func is the function to be called. Functions which have the same return
/// type and take the same number of arguments can generally use the same
/// helper function.
///
/// To export a function, add a corresponding entry to table
/// #aux_functions 
typedef struct aux_function
{
		/** The function to be exported */
	void *func; 
	
		/** A helper function which converts parameters and return value
		    to/from the interpreter's format*/
	void (*helper_func) (void *func, long *params, return_info_t *ret);
} aux_function;

//{{{
static sint tile_is_passable(sint x, sint y)
{
	// Edge tiles should not be treated as passable (to keep bolts and such
	// from going through them)
	if(x<=0 || y<=0 || x>=MAPSIZE_X-1 || y>=MAPSIZE_Y-1)
		return 0;
	return TILEDESC(w->t[y][x]).passable;
}
//}}}
//{{{
static sint tile_is_transparent(sint x, sint y)
{
	// Edge tiles should not be treated as transparent (to keep bolts and such
	// from going through them)
	if(x<=0 || y<=0 || x>=MAPSIZE_X-1 || y>=MAPSIZE_Y-1)
		return 0;
	return TILEDESC(w->t[y][x]).transparent;
}
//}}}
//{{{
static sint tile_is_occupied(sint x, sint y)
{
	return w->t[y][x].flags & TFLAG_OCCUPIED;
}
//}}}
//{{{
static void *get_plr(void)
{
	return &w->plr;
}
//}}}
//{{{
static void *get_w(void)
{
	return w;
}
//}}}
//{{{
static sint fstrcmp(char *one, char *two)
{
	return strcmp(one, two);
}
//}}}

//{{{
static void void_helper_void(void *func, long *params, return_info_t *ret)
{
	((void (*)(void))func)();
}
//}}}
//{{{
static void void_helper_pointer(void *func, long *params, return_info_t *ret)
{
	((void (*)(char *))func)((void*)params[0]);
}
//}}}
//{{{
static void void_helper_pointer_pointer(void *func, long *params, return_info_t *ret)
{
	((void (*)(char *, char*))func)((void*)params[0], (void*)params[1]);
}
//}}}
//{{{
static void pointer_helper_short(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = (long)(( long*(*)(ushort) )func)(params[0]);
}
//}}}
//{{{
static void int_helper_short(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = ((sint (*)(ushort))func)(params[0]);
}
//}}}
//{{{
static void int_helper_short_short(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = ((sint (*)(ushort,ushort))func)(params[0], params[1]);
}
//}}}
//{{{
static void int_helper_short_short_short(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = ((sint (*)(ushort,ushort,ushort))func)(params[0], params[1], params[2]);
}
//}}}
//{{{
static void void_helper_short(void *func, long *params, return_info_t *ret)
{
	((void (*)(ushort))func)(params[0]);
}
//}}}
//{{{
static void void_helper_short_short(void *func, long *params, return_info_t *ret)
{
	((void (*)(ushort,ushort))func)(params[0], params[1]);
}
//}}}
//{{{
static void void_helper_short_short_short(void *func, long *params, return_info_t *ret)
{
	((void (*)(ushort,ushort,ushort))func)(params[0], params[1], params[2]);
}
//}}}
//{{{
static void int_helper_pointer(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = ((ushort (*)(char *))func)((void*)params[0]);
}
//}}}
//{{{
static void int_helper_pointer_pointer(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = ((ushort (*)(char*, char*))func)((void*)params[0], (void*)params[1]);
}
//}}}
//{{{
static void int_helper_pointer_short(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = ((ushort (*)(char *,ushort))func)((void*)params[0], (ushort)params[1]);
}
//}}}
//{{{
static void int_helper_void(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = ((ushort (*)(void))func)();
}
//}}}
//{{{
static void int_helper_short_short_pointer_pointer_short(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval =
		((ushort (*)(ushort,ushort,char*,char*,ushort))func)(params[0], params[1], (void*)params[2], (void*)params[3], params[4]);
}
//}}}
//{{{
static void void_helper_pointer_vararg(void *func, long *params, return_info_t *ret)
{
	((void (*)(char *, va_list))func)((void*)params[0], (va_list)(params+1));
}
//}}}
//{{{
static void pointer_helper_pointer_vararg(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = (long)((char* (*)(char *, va_list))func)
	              ((void*)params[0], (va_list)(params+1));
}
//}}}
//{{{
static void int_helper_short_short_vararg(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = (long)((sint (*)(ushort, ushort, va_list))func)((int)params[0], (int)params[1], (va_list)(params+2));
}
//}}}
//{{{
static void pointer_helper_void(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = (long)((void* (*)(void))func)();
}
//}}}
//{{{
static void pointer_helper_long(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = (long)((void* (*)(long))func)(params[0]);
}
//}}}
//{{{
static void pointer_helper_pointer(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = (long)((void* (*)(char*))func)((char*)params[0]);
}
//}}}
//{{{
static void long_helper_void(void *func, long *params, return_info_t *ret)
{
	ret->returns = 1;
	ret->retval = ((ulong (*)(void))func)();
}
//}}}
//{{{
static void void_helper_long(void *func, long *params, return_info_t *ret)
{
	((void (*)(ulong))func)(params[0]);
}
//}}}
//{{{
static void void_helper_pointer_short_short(void *func, long *params, return_info_t *ret)
{
	((void (*)(char *, ushort, ushort))func)((void*)params[0], params[1], params[2]);
}
//}}}

/*
 * The contents of this table should match the prototypes in data/c_functions.m4
 */
static aux_function aux_functions[] =
	{
		{ vmessage,           void_helper_pointer_vararg    },
		{ nrandom,            int_helper_short_short        },
		{ plr_mod_stat,       void_helper_short_short       },
		{ base_coat,          void_helper_void              },
		{ place_room,         int_helper_pointer            },
		{ place_hole,         void_helper_void              },
		{ place_corridors,    int_helper_void               },
		{ place_doors,        void_helper_void              },
		{ fill_room,          void_helper_pointer           },
		{ UI_Dialog_Default,  void_helper_pointer           },
		{ make_connections,   void_helper_void              },
		{ place_items,        void_helper_void              },
		{ make_noise,         void_helper_short_short       },
		{ cave_generation,    void_helper_short_short       },
		{ prompt,             int_helper_pointer            },
		{ prompt_dir,         void_helper_pointer_pointer   },
		{ illuminate,         void_helper_short_short_short },
		{ calc_light,         void_helper_void              },
		{ attack_tile,        int_helper_short_short_pointer_pointer_short },
		{ delay,              void_helper_void              },
		{ UI_TF_DrawEffect,   void_helper_short_short_short },
		{ UI_TF_ClearEffects, void_helper_void              },
		{ monstbytile,        int_helper_short_short        },
		{ tile_is_passable,   int_helper_short_short        },
		{ tile_is_transparent,int_helper_short_short        },
		{ tile_is_occupied,   int_helper_short_short        },
		{ get_plr,            pointer_helper_void           },
		{ plr_restore_stat,   int_helper_short_short        },
		{ get_w,              pointer_helper_void           },
		{ player_gold,        long_helper_void              },
		{ debit_gold,         void_helper_long              },
		{ deref_file_ptr,     pointer_helper_long           },
		{ place_item,         void_helper_pointer_short_short },
		{ scare_monster,      void_helper_short             },
		{ update_player,      void_helper_void              },
		{ identify,           int_helper_void               },
		{ UI_Menu_Set_Persist, void_helper_short            },
		{ draw_tile,          void_helper_short_short       },
		{ entangle,           void_helper_short             },
		{ UI_MF_wait,         void_helper_void              },
		{ study_book,         int_helper_short_short_vararg },
		{ plr_moveto,         void_helper_short_short_short },
		{ give_item,          int_helper_pointer_short      },
		{ confuse,            void_helper_short             },
		{ hallucinate,        void_helper_short             },
		{ timepass,           void_helper_short             },
		{ identify_type,      void_helper_short             },
		{ addmonster_ptr,     void_helper_pointer_short_short },
		{ end_game,           void_helper_pointer           },
		{ die,                void_helper_pointer           },
		{ fix_rust,           int_helper_void               },
		{ monstname,          pointer_helper_short          },
		{ fstrcmp,            int_helper_pointer_pointer    },
		{ monst_heal,         void_helper_short_short       },
		{ numb,               void_helper_short             },
		{ detect_invisible,   void_helper_short             },
		{ place_player_randomly, void_helper_void           },
		{ removecurses,       void_helper_void              },
		{ enchant_item,       int_helper_void               },
		{ n_itemname,         pointer_helper_long           },
		{ monstmoveto,        void_helper_short_short_short },
		{ webspinner_build,   void_helper_short_short_short },
		{ webspinner_cower,   void_helper_short_short_short },
		{ monst_battle_pet,   int_helper_short              },
		{ monst_detect_player, int_helper_short             },
		{ monst_can_attack_player, int_helper_short         },
		{ monstmovetowardsplayer, void_helper_short         },
		{ monst_chat,         void_helper_short             },
		{ vretprintf,         pointer_helper_pointer_vararg },
		{ indefinite_article, pointer_helper_pointer        },
		{ paralyze,           void_helper_short             },
		{ rust_inventory,     void_helper_short             },
		{ monst_remove,       void_helper_short             },
		{ monst_facing,       int_helper_short              },
		{ player_can_see,     int_helper_short              },
		{ monst_takedamage,   void_helper_short_short_short },
		{ monstcanmove,       int_helper_short_short_short  },
		{ engulf_in_darkness, void_helper_short             },
		{ top_item,           int_helper_short_short        },
		{ mkmap_big_front,    int_helper_void               },
		{ cave_partial_gen,   void_helper_short_short       },
		{ full_redraw,        void_helper_void              },
		{ monst_anger,        void_helper_short             },
		{ confuse_monster,    void_helper_short             },
		{ follow_connection,  void_helper_void              },
		{ plr_step,           void_helper_short_short       }
	};

return_info_t interpret_call(int which_func, long *params)
{
	return_info_t ret;
	ret.returns = 0;
	
	aux_functions[which_func].helper_func( aux_functions[which_func].func,
	                                       params, &ret );
	
	return ret;
}

