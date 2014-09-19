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
// interpret.c
/// @file
/// @brief Bytecode interpreter

#include "crogue.h"
#ifdef INTERPRETER_DEBUG
	#include <assert.h>
#endif

#ifdef INTERPRETER_DEBUG
	#ifndef INLINE
		#define INLINE
	#endif
#else
	#ifndef INLINE
		#define INLINE inline
	#endif
#endif

//{{{
#define OP_NOP     0x00
#define OP_HALT    0x01
#define OP_DISC    0x02
#define OP_DUP     0x03
#define OP_ALLOCA  0x04
//}}}
//{{{
#define OP_RET     0x05
#define OP_CALL    0x06
#define OP_STA     0x07
#define OP_MVA     0x08
#define OP_LINK    0x09
#define OP_CCALL   0x0A
//}}}
//{{{
#define OP_LDR     0x0B
#define OP_LDR2    0x0C
#define OP_STR     0x0D
#define OP_MOV     0x0E
#define OP_LDIB    0x0F
#define OP_LDIW    0x10
#define OP_LDIL    0x11
#define OP_LDIZ    0x12
#define OP_LDIO    0x13
//}}}
//{{{
#define OP_ADD     0x14
#define OP_SUB     0x15
#define OP_MUL     0x16
#define OP_DIV     0x17
#define OP_MOD     0x18
#define OP_ASL     0x19
#define OP_ASR     0x1A
#define OP_AND     0x1B
#define OP_OR      0x1C
#define OP_XOR     0x1D
#define OP_NEG     0x1E
#define OP_INCR    0x1F
#define OP_DECR    0x20
//}}}
//{{{
#define OP_BOOL    0x21
#define OP_BOOL2   0x22
#define OP_NOT     0x23
#define OP_TEQ     0x24
#define OP_TLT     0x25
#define OP_TGT     0x26
//}}}
//{{{
#define OP_JMP     0x27
#define OP_BRA     0x28
//}}}
//{{{
#define OP_DEREFB  0x29
#define OP_DEREFW  0x2A
#define OP_DEREFL  0x2B
#define OP_POKEB   0x2C
#define OP_POKEW   0x2D
#define OP_POKEL   0x2E
#define OP_LDLBL   0x2F
//}}}

//{{{
typedef union instruction_args
{
	signed   char bytes[6];  // HACKHACK: This is 6 instead of 4 so that the
	                         // union as a whole won't be the size of a register
	                         // (TIGCC tries to cram it in one otherwise, with
	                         // disastrous results.)
	unsigned char ubytes[4];
	signed   short words[2];
	unsigned short uwords[2];
	signed   long dword;
} instruction_args_t;
//}}}

static const char *interpreter_call_stack[32];
static int interpreter_call_stack_pos = 0;

#define pop_stack() (*(--stack_pos))
#define push_stack(n) (*(stack_pos++) = (n))
#define write_arg(v) (*(arg_pos++) = (v))
#define load_register(n) (register_pos[n])
#define save_register(n,v) (register_pos[n] = (v))
#define start_args() (arg_pos = register_pos + register_used + 1)

//{{{
static const char *interp_func_name(const char *pos)
{
	pos--;
	do {
		pos--;
	} while(*pos != '\0');
	pos++;
	return pos;
}
//}}}

//{{{
//
// *** This function MUST be reentrant when exiting via OP_CCALL! ***
//
long run_program(const unsigned char *program_data, long start_pc, int num_params, va_list params)
{
	const unsigned char *pc;
	
	long *stack_pos;
	long *register_pos;
	long *arg_pos;
	short call_pos;
	short register_used;
	long active_stack[ACTIVE_STACK_DEPTH];
	long register_stack[REGISTER_STACK_DEPTH];
	const unsigned char *call_stack[CALL_STACK_DEPTH];
	const unsigned char *segment_stack[CALL_STACK_DEPTH];
	
	{ // Initialization
		int ii;
		
		pc = program_data+start_pc;
		register_used = 0;
		stack_pos = active_stack;
		register_pos = register_stack;
		call_pos = 0;
		segment_stack[0] = program_data;
		
		start_args();
		for(ii=0; ii<num_params; ii++)
			write_arg(va_arg(params, long));
		va_end(params);
	}
	
	while(1)
	{
		instruction_args_t args;
		
		args.bytes[0] = pc[1];
		args.bytes[1] = pc[2];
		
		switch(*(pc++))
		{
//{{{
		case OP_NOP: {
			break;
		}
		case OP_HALT: {
			return 0;
		}
		case OP_DISC: {
			(void)pop_stack();
			break;
		}
		case OP_DUP: {
			long param = pop_stack();
			push_stack(param);
			push_stack(param);
			break;
		}
		case OP_ALLOCA: {
			long param = pop_stack();
			push_stack( (long)(register_pos + register_used) );
			register_used += ((param+3) & (~3)) >> 2;
				// use size/4 rounded up registers
			start_args();
			break;
		}
//}}}
//{{{
		case OP_RET: {
			interpreter_call_stack_pos--;
			if(call_pos == 0)
			{
				if(stack_pos > active_stack)
					return pop_stack();
				else
					return 0;
			}
			
			// Unlink registers
			register_pos--;
			register_used = *register_pos;
			register_pos -= register_used;
			
			pc = call_stack[--call_pos];
			start_args();
			interpreter_call_stack[interpreter_call_stack_pos] = NULL;
			break;
		}
		case OP_CALL: {
			call_stack[call_pos++] = pc+3;
			segment_stack[call_pos] = w->const_file[ pc[2] ];
			pc = w->const_file[ pc[2] ] + args.uwords[0];
			break;
		}
		case OP_STA: {
			write_arg(pop_stack());
			break;
		}
		case OP_MVA: {
			write_arg(load_register(*pc));
			pc++;
			break;
		}
		case OP_LINK: {
			// Allocate registers
			register_pos[register_used] = register_used;
			register_pos += register_used + 1;
			register_used = *pc;
			
			start_args();
			interpreter_call_stack[interpreter_call_stack_pos++] =
				interp_func_name(pc-1);
			pc++;
			break;
		}
		case OP_CCALL: {
			return_info_t ret;
			pc += 2;
			// Call C interface function by index (see table in interpret_aux.c)
			ret = interpret_call(args.words[0],
			                     register_pos + register_used + 1);
			if(ret.returns)
				push_stack( ret.retval );
			start_args();
			break;
		}
//}}}
//{{{
		case OP_LDR2: {
			push_stack(load_register( *(pc++) ));
			push_stack(load_register( *(pc++) ));
			break;
		}
			
		case OP_LDR: {
			push_stack(load_register( *(pc++) ));
			break;
		}
		case OP_STR: {
			save_register(*(pc++), pop_stack());
			break;
		}
		case OP_MOV: {
			unsigned char source = *(pc++),
			              dest   = *(pc++);
			save_register(dest, load_register(source));
			break;
		}
		case OP_LDIB: {
			push_stack((long)( *((const char*)pc++) ));
				// Cast away the unsigned because immediate values *are* signed
			break;
		}
		case OP_LDIW: {
			pc += 2;
			push_stack((long)(args.words[0]));
			break;
		}
		case OP_LDIL: {
			pc+=2;
			args.bytes[2] = *(pc++);
			args.bytes[3] = *(pc++);
			push_stack( args.dword );
			break;
		}
		case OP_LDIZ: {
			push_stack(0);
			break;
		}
		case OP_LDIO: {
			push_stack(1);
			break;
		}
//}}}
//{{{
		case OP_ADD: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			push_stack(param1+param2);
			break;
		}
		case OP_SUB: {
			long param2 = pop_stack();
			long param1 = pop_stack();
			push_stack(param1 - param2);
			break;
		}
		case OP_MUL: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			push_stack(param1 * param2);
			break;
		}
		case OP_DIV: {
			long param2 = pop_stack();
			long param1 = pop_stack();
			push_stack(param1 / param2);
			break;
		}
		case OP_MOD: {
			long param2 = pop_stack();
			long param1 = pop_stack();
			push_stack(param1 % param2);
			break;
		}
		case OP_ASL: {
			long param2 = pop_stack();
			long param1 = pop_stack();
			push_stack(param1 << param2);
			break;
		}
		case OP_ASR: {
			long param2 = pop_stack();
			long param1 = pop_stack();
			push_stack(param1 >> param2);
			break;
		}
		case OP_AND: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			push_stack(param1 & param2);
			break;
		}
		case OP_OR: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			push_stack(param1 | param2);
			break;
		}
		case OP_XOR: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			push_stack(param1 ^ param2);
			break;
		}
		case OP_NEG: {
			long param = pop_stack();
			push_stack(-param);
			break;
		}
		case OP_INCR: {
			save_register(*pc, load_register(*pc) + 1);
			pc++;
			break;
		}
		case OP_DECR: {
			save_register(*pc, load_register(*pc) - 1);
			pc++;
			break;
		}
//}}}
//{{{
		case OP_BOOL: {
			long param = pop_stack();
			push_stack(!!param);
			break;
		}
		case OP_BOOL2: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			push_stack(!!param2);
			push_stack(!!param1);
			break;
		}
		case OP_NOT: {
			long param = pop_stack();
			push_stack(!param);
			break;
		}
		case OP_TEQ: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			push_stack(param1 == param2);
			break;
		}
		case OP_TLT: {
			long param2 = pop_stack();
			long param1 = pop_stack();
			push_stack(param1 < param2);
			break;
		}
		case OP_TGT: {
			long param2 = pop_stack();
			long param1 = pop_stack();
			push_stack(param1 > param2);
			break;
		}
//}}}
//{{{
		case OP_JMP: {
			pc = segment_stack[call_pos] + args.uwords[0];
			break;
		}
		case OP_BRA: {
			if(pop_stack()) {
				pc = segment_stack[call_pos] + args.uwords[0];
			} else {
				pc += 2;
			}
			break;
		}
//}}}
//{{{
		case OP_DEREFB: {
			long param = pop_stack();
			#ifdef INTERPRETER_DEBUG
				assert(param > 1024); // Check for null or near-null ptrs
			#endif
			push_stack( *((char*)param) );
			break;
		}
		case OP_DEREFW: {
			long param = pop_stack();
			#ifdef INTERPRETER_DEBUG
				assert(param > 1024);
			#endif
			push_stack( *((short*)param) );
			break;
		}
		case OP_DEREFL: {
			long param = pop_stack();
			#ifdef INTERPRETER_DEBUG
				assert(param > 1024);
			#endif
			push_stack( *((long*)param) );
			break;
		}
		case OP_POKEB: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			#ifdef INTERPRETER_DEBUG
				assert(param2 > 1024);
			#endif
			*((char*)param2) = param1;
			break;
		}
		case OP_POKEW: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			#ifdef INTERPRETER_DEBUG
				assert(param2 > 1024);
			#endif
			*((short*)param2) = param1;
			break;
		}
		case OP_POKEL: {
			long param1 = pop_stack();
			long param2 = pop_stack();
			#ifdef INTERPRETER_DEBUG
				assert(param2 > 1024);
			#endif
			*((long*)param2) = param1;
			break;
		}
		case OP_LDLBL: {
			args.ubytes[2] = pc[2];
			pc += 3;
			push_stack((long)(w->const_file[args.ubytes[2]] + args.uwords[0]));
			break;
		}
//}}}
//{{{
		default:
			break;
//}}}
		}
	}
}
//}}}

