// interpret.h
/// @file
/// @brief Bytecode interpretter
/// @sa src/interpret.c

#ifndef INTERPRET_H
#define INTERPRET_H

#include "substdio.h"

#define ACTIVE_STACK_DEPTH 64
#define REGISTER_STACK_DEPTH 256
#define CALL_STACK_DEPTH 32

//{{{
typedef struct return_info
{
	short returns;
	long retval;
} return_info_t;
//}}}

void load_and_run(char *filename);
long run_program(const unsigned char *program_data, long start_pc, int num_params, va_list params);
return_info_t interpret_call(int which_func, long *params);

#endif //INTERPRET_H

