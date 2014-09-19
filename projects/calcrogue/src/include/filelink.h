// struct_filelink.h
/// @file
/// @brief Links inside the data file
/// @sa util.c
/// @sa deref_file_ptr

#ifndef STRUCT_FILELINK_H
#define STRUCT_FILELINK_H

/// @brief A reference to a location within the data file
///
/// @sa deref_file_ptr deref_file_ptr_partial
struct filelink
{
		/// Type of reference. May be one of @ref linktypes "link types".
	schar type; 
		/// Which segment the target is in.
	uchar segment;
		/// Offset into the file
	uint offset;
};
TYPEDEF(struct filelink filelink)

/// @defgroup linktypes Link types
/// Possible values for #filelink::type. 
/// @{
	/// Dereferencing produces a pointer to some data in the file
#define PTR_NORMAL     0
	/// Target is a jump table; the result is randomized when dereferencing
#define PTR_JUMPTABLE  1
	/// Target is a shuffle table; the result is randomized once per game
#define PTR_SHUFFLETAB 2
	/// Target is a function in the DLL; dereferencing calls the function
#define PTR_DLLCALL    3
	/// Target is an interpreted function; dereferencing calls the function
#define PTR_INTERPCALL 4
/// @}

#define jt_unconditional 0
#define jt_playerlevel   1
#define jt_dungeonlevel  2
#define jt_avglevel      3
#define jt_class         4

#ifndef DATA_FILE
extern ECONST filelink filelink_null;
#endif

#endif //STRUCT_FILELINK_H

