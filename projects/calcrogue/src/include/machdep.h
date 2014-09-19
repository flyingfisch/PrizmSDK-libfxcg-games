// machdep.h
/// @file
/// @brief Target-specific stuff

#ifndef MACHDEP_H
#define MACHDEP_H

#ifndef TARGET
#  error No target specified
#endif

#define T_TI89        1
#define T_TI92P       2
#define T_WIN32_MSC   3
#define T_UNIX        4
#define T_WIN32_MINGW 5
#define T_DOXYGEN     4  /* Not for actual compilation, but for documentation.
                            Will pretend to be Unixy */
#define T_CPROTO      6  /* Not for actual compilation, but for extracting
                            prototypes. */
#define T_PALMOS      7

#define ENGLISH 0
#define FRENCH 1
#define CZECH 2

//{{{
#if TARGET == T_TI89
#	define IS_CALCULATOR
#	define USE_TILES
#	define USE_TIGCCLIB
#	define USE_TI89
#endif
//}}}
//{{{
#if TARGET == T_TI92P
#	define IS_CALCULATOR
#	define USE_TILES
#	define USE_TIGCCLIB
#	define USE_TI92P
#	define USE_V200
#endif
//}}}
//{{{
#if TARGET == T_WIN32_MSC
#	define REALCOMPUTER
#	define WIN32
#	define WINDOWS
#	define WIN32_MSC
#	define USE_CURSES
#	define SUPPORT_COLOR
#endif
//}}}
//{{{
#if TARGET == T_WIN32_MINGW
#	define REALCOMPUTER
#	define WINDOWS
#	define WIN32_MINGW
#	define USE_CURSES
#	define SUPPORT_COLOR
#endif
//}}}
//{{{
#if TARGET == T_UNIX
#	define REALCOMPUTER
#	define UNIX
#	define EXTERNAL_COMPRESSOR "/usr/bin/gzip"
#	define USE_CURSES
#	define SUPPORT_COLOR
#	define SHARED_SCORES
#	include <sys/unistd.h>
#	include <sys/stat.h>
#endif
//}}}
//{{{
#if TARGET == T_PALMOS
#	include <PalmOS.h>
#	include <PalmOSGlue/TxtGlue.h>
#	include <PalmOSGlue/WinGlue.h>
#	include "resource.h"
#	define PALMOS
#	define IS_PALM
#	define exit PalmExit
#	define CONSERVE_STACK
#	define SUPPORT_COLOR
	void PalmExit(int code);
#endif
//}}}
//{{{
#if TARGET == T_CPROTO
#	define REALCOMPUTER
#	define SUPPORT_COLOR
#	include "cproto.h"
#endif
//}}}

#include "configure.h"


#ifdef REALCOMPUTER
#	undef REGISTERVAR // Can't specify m68k registers on a PC
#endif

//{{{
#ifdef REALCOMPUTER
#	include <stdio.h>
#	include <stdlib.h>
#	include <time.h>
#	include <stdarg.h>
#	include <ctype.h>
#	include <string.h>
#endif

#ifdef USE_CURSES
int curses_title_screen(void);
#endif
//}}}

#define TYPEDEF(a) typedef a;
//{{{
// Typedefs for brevity (and for length compatibility between 16- and 32-bit
// int sizes).
#define uchar  unsigned char
#define schar  signed char
#define ushort unsigned short
#define sshort signed short
#define uint   unsigned short
#define sint   signed short
#define ulong  unsigned long
#define slong  signed long
//}}}

#include "platform.h"

//{{{
// GCC extensions
#ifdef __GNUC__
#	define ATTRIB(a) __attribute__((a))
#	define CASTCONSTRUCT
#	define HAVE_ALLOCA
#else
#	define ATTRIB(a)
#endif
//}}}
//{{{
#ifdef alloca
#	define HAVE_ALLOCA
#endif
//}}}
//{{{
// MSC BS workarounds
#ifdef _MSC_VER
#	pragma warning ( disable : 4761 ) // Suppress integral size mismatch warnings
#define INLINE // inline keyword flat-out doesn't compile with MS's C compiler!
#define snprintf _snprintf
#else
#define INLINE inline
#endif
//}}}

// DLL handling/emulation
#ifndef IS_CALCULATOR
const void** dll_load(const void *f);
#else
#	define OPTIMIZE_ROM_CALLS
                            ///< (Calculator) Special calling convention for
                            /// size optimization (Handled by library headers)

#	define EXECUTE_IN_GHOST_SPACE
                            ///< (For TI calculators) Execute in a memory ghost
                            /// space, OR verify that HW3patch is installed (if
                            /// needed)

#endif

#include "positions.h"

#ifdef DISABLE_COMPRESSION
#define DISABLE_HUFFMAN
#define DISABLE_RLE
#endif

#endif //MACHDEP_H

