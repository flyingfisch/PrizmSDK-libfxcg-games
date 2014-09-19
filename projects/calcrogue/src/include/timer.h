// timer.h
/// @file
/// @brief Timers
/// @sa src/timers.c
/// @sa include/struct_timer.h

#ifndef TIMER_H
#define TIMER_H

#define TIMER_STAT   0
#define TIMER_FUNC   1

//{{{
struct stat_timer_desc
{
	uint type;
	struct stat_mod_desc effect;
	CONST char *expire_message;
};
TYPEDEF(struct stat_timer_desc stat_timer_desc)
//}}}
//{{{
struct func_timer_desc
{
	uint type;
#ifndef DATA_FILE
	void (*apply)(void);
	void (*expire)(void);
#else
	void *apply;
	void *expire;
#endif
};
TYPEDEF(struct func_timer_desc func_timer_desc)
//}}}
//{{{
union timer_desc
{
	struct stat_timer_desc stat;
	struct func_timer_desc func;
};
TYPEDEF(union timer_desc timer_desc)
//}}}
//{{{
struct stat_timer
{
	uint type;
	uint padding;
	CONST union timer_desc *desc;
	ulong expiry;
};
//}}}
TYPEDEF(struct stat_timer stat_timer)

#endif //TIMER_H

