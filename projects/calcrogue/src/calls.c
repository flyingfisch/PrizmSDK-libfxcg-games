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
// calls.c
/// @file
/// @brief Interface for calling bytecode-interpretted functions referenced
/// in the data file.
///
/// Each function here corresponds to a function type for interpretted code.

#include "crogue.h"

//{{{
void call_stepfunc(filelink f, uint x, uint y)
{
	long args[2];
	
	if(isNull(f)) return;
	f = deref_file_ptr_partial(f);
	
	args[0] = x;
	args[1] = y;
	
	if(f.type == PTR_INTERPCALL)
		run_program(w->const_file[f.segment], f.offset, 2, (va_list)args);
	else
		((void (*)(uint x, uint y))(w->dll_functions[f.offset]))(x, y);
}
//}}}
//{{{
void call_genericfunc(filelink f)
{
	if(isNull(f)) return;
	f = deref_file_ptr_partial(f);
	
	if(f.type == PTR_INTERPCALL)
		run_program(w->const_file[f.segment], f.offset, 0, (va_list)NULL);
	else
		((void (*)(void))(w->dll_functions[f.offset]))();
}
//}}}
//{{{
void call_throwhitfunc(filelink f, int monst)
{
	long arg = monst;
	
	if(isNull(f)) return;
	f = deref_file_ptr_partial(f);
	
	if(f.type == PTR_INTERPCALL)
		run_program(w->const_file[f.segment], f.offset, 1, (va_list)&arg);
	else
		((void (*)(int monst))(w->dll_functions[f.offset]))(monst);
}
//}}}
//{{{
int call_usefunc(filelink f, int which_item)
{
	long arg = which_item;
	
	if(isNull(f)) return 0;
	f = deref_file_ptr_partial(f);
	
	if(f.type == PTR_INTERPCALL)
		return run_program(w->const_file[f.segment], f.offset, 1, (va_list)&arg);
	else
		return ((int (*)(int))(w->dll_functions[f.offset]))(which_item);
}
//}}}
//{{{
int call_attackfunc(filelink f, uint monst, uint damage, sint target)
{
	long args[3];
	
	if(isNull(f)) return 0;
	f = deref_file_ptr_partial(f);
	
	args[0] = monst;
	args[1] = damage;
	args[2] = target;
	
	if(f.type == PTR_INTERPCALL)
		return run_program(w->const_file[f.segment], f.offset, 3, (va_list)args);
	else
		return ((int (*)(uint, uint, sint))
			(w->dll_functions[f.offset]))
			(monst, damage, target);
}
//}}}

//{{{
long call_generic(int index, int num_args, ...)
{
	va_list args;
	const filelink *table = deref_file_ptr(w->desc.function_entries);
	filelink function = table[index];
	va_start(args, num_args);
	return run_program(w->const_file[function.segment], function.offset,
	                   num_args, args);
}
//}}}

