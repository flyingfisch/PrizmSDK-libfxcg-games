// util.h
/// @file
/// @brief Miscellaneous useful functions

#ifndef UTIL_H
#define UTIL_H

//{{{
/// @brief A coordinate
///
/// Ranges from (0,0) to (255,255). This is sufficient to represent any spot
/// on the map, since no maps are that big, and (due to memory limitations)
/// no maps will ever be that big. This may also represent a location on the
/// screen, since the screen size (in pixels or in characters) is never that
/// big, either.
struct coord
{
	uchar x;
	uchar y;
};
TYPEDEF(struct coord coord)
//}}}
//{{{
/// @brief A (signed short) coordinate
struct scoord
{
	short x;
	short y;
};
TYPEDEF(struct scoord scoord)
//}}}
//{{{
/// @brief A rectangle
///
/// Start and size range from (0,0) to (255,255). This is sufficient to
/// represent any drawable rectangle on the screen, or any region on the
/// map.
struct rect
{
	struct coord topleft; ///< Top-left corner
	struct coord extent;  ///< Width and height
};
TYPEDEF(struct rect rect)
//}}}
//{{{
/// @brief A heading (one of eight cardinal directions)
///
/// Given as a vector of the form ([-1,0,1], [-1,0,1]). To iterate in the
/// specified direction, simply step in (x,y). Note that 'in place' is a
/// valid direction (x=0, y=0). \sa facing.c
struct direction
{
	schar x;
	schar y;
};
TYPEDEF(struct direction direction)
//}}}
//{{{
/// @brief A numeric range
///
/// A numeric range given as [min, max]. This may be used to generate a
/// random number on the closed interval with rrandom(). Note that only
/// nonnegative numbers from 0 to 255 are allowed. For a numeric range
/// with large and/or negative numbers allowed, use #lrange.
struct range
{
	uchar max;
	uchar min;
};
TYPEDEF(struct range range)
//}}}
//{{{
/// @brief A numeric range
///
/// A numeric range given as [min, max]. Min and max range over the entire
/// range of signed integers.
struct lrange
{
	sint max;
	sint min;
};
TYPEDEF(struct lrange lrange)
//}}}
//{{{
// Cast constructors workaround
#ifdef CASTCONSTRUCT
#	define COORD(a,b) ((coord){(a),(b)})
#	define RECT(a,b,c,d) ((rect){{a,b},{c,d}})
#	define SRANGE(a,b) ((range){(a),(b)})
#	define DIRECTION(a,b) ((direction){(a),(b)})
#else
#	define COORD(a,b) construct_coord(a, b)
#	define RECT(a,b,c,d) construct_rect(a,b,c,d)
#	define SRANGE(a,b) construct_range((a),(b))
#	define DIRECTION(a,b) (construct_direction((a),(b)))
	struct coord construct_coord(uchar x, uchar y);
	struct rect construct_rect(uchar x, uchar y, uchar w, uchar h);
	struct direction construct_direction(schar x, schar y);
	struct range construct_range(uchar max, uchar min);
#endif
//}}}

#ifdef SUPPORT_COLOR
//{{{
struct colorinfo
{
	uchar dark;
	uchar lit;
};
TYPEDEF(struct colorinfo colorinfo)

#define COLOR_BOLD 0x10
#define BLACK    0
#define RED      1
#define GREEN    2
#define YELLOW   3
#define BLUE     4
#define MAGENTA  5
#define CYAN     6
#define WHITE    7
#define LBLACK   16
#define LRED     17
#define LGREEN   18
#define LYELLOW  19
#define LBLUE    20
#define LMAGENTA 21
#define LCYAN    22
#define LWHITE   23
//}}}
#else
TYPEDEF(void *colorinfo)
#endif

#include "filelink.h"

    // Better to pass as high and low bytes of one arg because most calls use
    // constant (precalculatable) values
#define RANGE(a,b) xrandom( ((a)<<8)+(b) )

#define distancesquare(x1, y1, x2, y2) ( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) )

#ifndef IS_CALCULATOR
#ifndef DATA_FILE
#	ifdef min // Some platforms provide broken versions of this
#		undef min
#	endif
	int min(int a, int b);
#	ifdef max
#		undef max
#	endif
	int max(int a, int b);
#endif
#endif

/* Helpers are provided by effect, but what really matters is calling
   convention. Thus, some aliases for equivalent calling conventions: */
#define call_aifunc call_usefunc
#define call_chatfunc call_throwhitfunc
#define call_killfunc call_throwhitfunc

/* drawstring.c */
/// @brief A saved cursor position (for use with #draw_string)
///
/// Initialize to {0,0} and use only as a parameter to #draw_string.
struct draw_string_info
{
	int curX;
	int curRow;
};
TYPEDEF(struct draw_string_info draw_string_info)

#ifndef DATA_FILE
#ifdef DEBUG_ALLOCATOR
int debug_verify_mem(void);
int debug_verify_block(long *block);
void *debug_malloc(size_t S);
void debug_free(void *ptr);
void *debug_calloc(size_t N, size_t S);
void *debug_realloc(void *ptr, size_t S);
#else // DEBUG_ALLOCATOR
#	define debug_malloc  malloc_throw
#	define debug_free    free
#	define debug_calloc  calloc_throw
#	define debug_realloc realloc_throw
#endif // DEBUG_ALLOCATOR
#endif // DATA_FILE

#endif //UTIL_H

