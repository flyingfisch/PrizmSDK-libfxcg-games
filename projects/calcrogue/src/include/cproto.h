// cproto.h
/// @file
/// @brief Workaround for  limitations of cproto's parsing

#ifndef CPROTO_H
#define CPROTO_H

#ifdef CPROTO
#	undef __GNUC__
#	define __extension__
#endif

#endif

