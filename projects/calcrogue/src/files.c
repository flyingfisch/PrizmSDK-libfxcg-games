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
// file.c
/// @file
/// @brief Management of saved games, saved levels, and the data file.

#include "crogue.h"
#include "export.h"

static const uchar save_file_extension[6] = { 0,'S','A','V',0,0xf8 };
static void cleanup_checkpoint(void);
static const char *savefile_name(void);
static INLINE const char *savefile_name(void);
static INLINE const char *savefile_name_uncompressed(void);
static INLINE const char *checkfile_name(void);

#ifdef IS_CALCULATOR
//{{{
static void archive_file(const char *filename, int filesize)
{
	ulong space_free, block_free;
	
	filesize += 16; // Margin of error for various overheads.
	
	EM_survey(NULL, &space_free, &block_free, NULL, NULL, NULL);
	space_free += block_free;
	
	if(space_free < filesize) {
		message(gettext("WARNING: Not enough archive free. Using main memory."));
		read_char();
		return;
	} else if(block_free < filesize) {
		if(prompt(gettext("Garbage collect archive memory?")))
		{
			message(gettext("Garbage collecting archive memory."));
			EM_GC(FALSE);
			EM_survey(NULL, &space_free, &block_free, NULL, NULL, NULL);
			if(space_free < filesize)
			{
				message(gettext("Still not enough archive free. Using main memory."));
				return;
			}
		} else {
			message(gettext("Not enough archive free. Using main memory."));
			read_char();
			return;
		}
	}
	EM_moveSymToExtMem(SYMSTR(filename), HS_NULL);
}
//}}}
//{{{
/// If [filename] exists, delete it, unarchiving if necessary.
static void unarchive_remove(const char *filename)
{
	EM_moveSymFromExtMem(SYMSTR(filename), HS_NULL);
	SymDel(SYMSTR(filename));
}
//}}}
#else
#	define unarchive_remove(filename) remove(filename)
#endif

//{{{
void link_data_file(void)
{
	w->tiledescs     = (struct tiledesc* )deref_file_ptr(w->desc.tileoffset);
	w->itemdescs     = (struct filelink* )deref_file_ptr(w->desc.itemoffset);
	w->spelldescs    = (struct spelldesc*)deref_file_ptr(w->desc.spelllist);
	w->shopdescs     = (struct shopdesc* )deref_file_ptr(w->desc.shopdescs);
	w->playerclasses = (struct classdesc*)deref_file_ptr(w->desc.playerclasses);
	w->shuffledata   = (struct filelink* )deref_file_ptr(w->desc.stdata);
#ifdef IS_CALCULATOR
	w->dll_functions = _DLL_call(const void**,(const import_functions *f),0)(&body_functions);
	w->dll_interface = (const export_functions*)w->dll_functions;
#else
	w->dll_functions = dll_load(&body_functions);
	w->dll_interface = (const export_functions*)w->dll_functions;
#endif
}
//}}}

//{{{
/// Save the game state.
void save_game(void)
{
	const char *filename;
#ifdef EXTERNAL_COMPRESSOR
	char cmd[512];
#endif
	
	filename = savefile_name_uncompressed();
	save_to_file(filename);
#ifdef EXTERNAL_COMPRESSOR
	sprintf(cmd, EXTERNAL_COMPRESSOR " -f -q %s", filename);
	system(cmd);
#endif
	
	// After a real save, the checkpoint save is no longer necessary.
	cleanup_checkpoint();
}
//}}}
//{{{
/// Save the game state.
void save_checkpoint(void)
{
	const char *filename;
	
	filename = checkfile_name();
	save_to_file(filename);
}
//}}}
//{{{
/// Save the game state.
void save_to_file(const char *filename)
{
	FILE *sg;
#ifdef IS_CALCULATOR
	int filesize;
#endif
	
	// (Calc) If a previous version of this save exists, delete it before
	// writing new one
#ifdef IS_CALCULATOR
	EM_moveSymFromExtMem(SYMSTR(filename), HS_NULL);
	SymDel(SYMSTR(filename));
#endif
	
	sg = fopen(filename, "wb");
	
	if(sg)
	{
		// Set up compressed output
		frleinit_write(sg, &fhuffmanputc);
		fhuffmaninit_write();
		
		// Write version number first
		putc(CROGUE_VERSION_MAJOR, sg);
		putc(CROGUE_VERSION_MINOR, sg);
		
		// Write the game state
		frlewrite(w, sizeof(world), sizeof(monster), sg);
		frlewrite(&w->t[0][0], sizeof(tile)*w->mapsize_x*w->mapsize_y,
			sizeof(tile), sg);
		frlewrite(w->shuffletranslation, sizeof(uint)*w->desc.shuffletabsize,
			sizeof(uint), sg);
		frlewrite(w->itemids, (w->desc.itementries/32 + 1) * 4, 1, sg);
		frlewrite(w->plr.spellknowledge, sizeof(slong)*w->desc.numspells,
			sizeof(slong), sg);
		frlewrite(w->items.items, sizeof(item)*w->items.alloced,
			sizeof(item), sg);
		if(w->wandering_monsters_num)
			frlewrite(w->wandering_monsters,
				sizeof(monster)*w->wandering_monsters_num, sizeof(monster), sg);
		frleflush(sg);
		fhuffmanflush(sg);
#ifdef IS_CALCULATOR
		fwrite(save_file_extension, 6, 1, sg);
		filesize = peek_w(sg->base);
#endif
		frlefinalize(sg);
		fclose(sg);
#ifdef IS_CALCULATOR
			if(get_option(OPTION_AUTOARCHIVE) == OPTION_ARCHIVE_YES)
				archive_file(filename, filesize);
#endif
	}
}
//}}}
//{{{
/// Attempts to load a saved game from the filesystem.
/// Returns 1 if successful or 0 if no saved game exists.
int restore_game(void)
{
	FILE *fin;
	short is_checkpoint = 0;
	
#ifdef EXTERNAL_COMPRESSOR
	char filename[512];
#else
	const char *filename;
#endif
	
	// Attempt to load a checkpoint-save first, because if there is one, it's
	// certainly more recent than the "real" save. Checkpoints never use an
	// external compressor, so there's no need to worry about that.
	fin = fopen(checkfile_name(), "rb");
	if(fin)
	{
		fclose(fin); // That was just to see if the file was there, so (slightly
		             // hackish), just close it for now.
		is_checkpoint = 1;
		#ifdef EXTERNAL_COMPRESSOR
			strcpy(filename, checkfile_name());
		#else
			filename = checkfile_name();
		#endif
	}
	else
	{
#ifdef EXTERNAL_COMPRESSOR
		strcpy(filename, savefile_name_uncompressed());
		system( retprintf(EXTERNAL_COMPRESSOR " -f -q -d %s.gz", filename) );
#else
		filename = savefile_name_uncompressed();
#endif
	}
	
	// (Palm) Rename the file to a name that won't be recognized before opening
	// it, so that if this crashes the calc, the game won't try to reopen the
	// same save on startup.
#ifdef PALMOS
	unarchive_remove(TEMPFILENAME);
	if(rename(filename, TEMPFILENAME) != 0)
		return 0;
	filename = TEMPFILENAME;
#endif
	
	fin = fopen( filename, "rb");
	if( !fin )
		return 0;
	
	if(!frleverify(fin))
	{
		message(gettext("There's a corrupted save file here..."));
		fclose(fin);
		read_char();
		return 0;
	}
	frleinit_read(fin);
	fhuffmaninit_read();
	
	// Check version number
	if(getc(fin)!=CROGUE_VERSION_MAJOR || getc(fin)!=CROGUE_VERSION_MINOR)
	{
		message(gettext("This saved game is from a different version."));
		fclose(fin);
		read_char();
		return 0;
	}
	
	frleread(w, sizeof(world), sizeof(monster), fin);
	w->t = NULL;
	resize_map_store(w->mapsize_x, w->mapsize_y);
	frleread(&w->t[0][0], sizeof(tile)*w->mapsize_x*w->mapsize_y,
		sizeof(tile), fin);
	sys_init_data_file();
	
	w->shuffletranslation = (uint*)debug_malloc(
		sizeof(uint) * w->desc.shuffletabsize );
	init_discoveries();
	w->items.items = (item*)debug_malloc(w->items.alloced*sizeof(item));
	init_spells();
	
	frleread(w->shuffletranslation, sizeof(uint)*w->desc.shuffletabsize, sizeof(uint), fin);
	frleread(w->itemids, (w->desc.itementries/32 + 1) * 4, 1, fin);
	frleread(w->plr.spellknowledge, sizeof(slong)*w->desc.numspells, sizeof(slong), fin);
	frleread(w->items.items, sizeof(item)*w->items.alloced, sizeof(item), fin);
	if(w->wandering_monsters_num)
	{
		w->wandering_monsters = debug_malloc(sizeof(monster)*w->wandering_monsters_num);
		frleread(w->wandering_monsters, sizeof(monster)*w->wandering_monsters_num, sizeof(monster), fin);
	}
	
	w->current_map = deref_file_ptr(w->current_map_link);
	
	fclose(fin);
#ifdef EXTERNAL_COMPRESSOR
	// Recompress save
	if(!is_checkpoint)
		system( retprintf(EXTERNAL_COMPRESSOR " -f -q %s", filename) );
#endif
#ifdef PALMOS
	unarchive_remove(TEMPFILENAME);
#endif
	return 1;
}
//}}}

const uchar level_file_extension[6] = { 0,'L','E','V',0,0xf8 };

//{{{
static INLINE const char *savefile_name(void)
{
#ifdef SHARED_SCORES
	#ifdef EXTERNAL_COMPRESSOR
	return retprintf("%s%s_%s.gz", file_prefix, getlogin(), SAVEFILENAME);
	#else
	return retprintf("%s%s_%s", file_prefix, getlogin(), SAVEFILENAME);
	#endif
#else
	return SAVEFILENAME;
#endif
}
//}}}
//{{{
static INLINE const char *savefile_name_uncompressed(void)
{
#ifdef SHARED_SCORES
	return retprintf("%s%s_%s", file_prefix, getlogin(), SAVEFILENAME);
#else
	return SAVEFILENAME;
#endif
}
//}}}
//{{{
static INLINE const char *checkfile_name(void)
{
#ifdef SHARED_SCORES
	return retprintf("%s%s_%s", file_prefix, getlogin(), CHECKFILENAME);
#else
	return CHECKFILENAME;
#endif
}
//}}}
//{{{
const char *levelfile_name(int levelnum)
{
#ifdef IS_CALCULATOR
	return retprintf("ROGLVL%i", levelnum);
#endif
#ifdef REALCOMPUTER
	#ifdef SHARED_SCORES
	return retprintf("%s%s_roglvl%i.sav", file_prefix, getlogin(), levelnum);
	#else
	return retprintf("roglvl%i.sav", levelnum);
	#endif
#endif	
#ifdef PALMOS
	return retprintf("roglvl%i", levelnum);
#endif
}
//}}}
#ifdef EXTERNAL_COMPRESSOR
//{{{
static const char *levelfile_name_compressed(int levelnum)
{
#ifdef SHARED_SCORES
	return retprintf("%s%s_roglvl%i.sav.gz", file_prefix, getlogin(), levelnum);
#else
	return retprintf("roglvl%i.sav.gz", levelnum);
#endif
}
//}}}
#endif
//{{{
//
// Save the current level (the parts of the game state which should be restored
// when the player comes back: items, monsters, walls/floor, etc). The two large
// tables (monsters and tiles) are written using a special form of run-length
// encoding. The first byts of each struct are ordered sequentially, followed
// by the second byte of each struct, etc. This avoids the complexity of having
// a single run length handle more than one character, while keeping similar
// bytes together.
//
// File naming convention depends on platform.
//
void save_level(void)
{
	FILE *save;
	const char *name;
	ushort map_size[2];
#	ifdef EXTERNAL_COMPRESSOR
	char cmd[512];
#	endif
#	ifdef IS_CALCULATOR
	int filesize;
#	endif
	
	name = levelfile_name(w->level);
	
#ifdef IS_CALCULATOR
	// Unlink pre-existing level of the same name
	EM_moveSymFromExtMem(SYMSTR(name), HS_NULL);
	SymDel(SYMSTR(name));
#endif
	
	// Prep for efficient writing
	compress_items();
	compress_monsters();
	
	save = fopen(name, "wb");
	
	if(!save) {
		message("Failed to open file when saving the level.");
		return;
	}
	
	frleinit_write(save, &fhuffmanputc);
	fhuffmaninit_write();
	
	// Write version number first
	putc(CROGUE_VERSION_MAJOR, save);
	putc(CROGUE_VERSION_MINOR, save);
	
	frlewrite( (char*)(&w->m[0]), sizeof(monster)*MONSTERS_MAX, sizeof(monster), save );
	map_size[0] = w->mapsize_x; map_size[1] = w->mapsize_y;
	frlewrite( (char*)(map_size), sizeof(ushort)*2, sizeof(ushort), save );
	frlewrite( (char*)(&w->t[0][0]), sizeof(tile)*MAPSIZE_X*MAPSIZE_Y, sizeof(tile), save );
	
	// Write items
	frlewrite(&w->items, sizeof(itemsinfo), 1, save);
	frlewrite(w->items.items, sizeof(item)*w->items.alloced, sizeof(item), save);
	
	frleflush(save);
	fhuffmanflush(save);
	
#	ifdef IS_CALCULATOR
		fwrite(level_file_extension, 6, 1, save);
		filesize = peek_w(save->base);
#	endif
	frlefinalize(save);
	fclose(save);

#	ifdef EXTERNAL_COMPRESSOR
		sprintf(cmd, EXTERNAL_COMPRESSOR " -f -q %s", name);
		system(cmd);
#	endif
#	ifdef IS_CALCULATOR	
		if(get_option(OPTION_AUTOARCHIVE) == OPTION_ARCHIVE_YES)
			archive_file(name, filesize);
#	endif
}
//}}}
//{{{
//
// Load the level number given in w->level from a run-length encoded file.
// Returns 1 if successful, 0 if the file does not exist.
//
int load_level(int source, coord pos)
{
	FILE *save;
	const char *name;
	ushort map_size[2];
#ifdef EXTERNAL_COMPRESSOR
	char cmd[512];
#endif
	
	name = levelfile_name(w->level);
	
#ifdef EXTERNAL_COMPRESSOR
	// Check for file existance and accessibility
	sprintf(cmd, "%s.gz", name);
	if(access(cmd, R_OK) != 0)
		return 0;
	
	// Open file
	sprintf(cmd, EXTERNAL_COMPRESSOR " -f -q -d %s.gz", name);
	system(cmd);
#endif
	
	save = fopen(name, "rb");
	
	if( !save )
		return 0;
	
	if(!frleverify(save))
	{
		message(gettext("This level is corrupt... I'll build a new one."));
		fclose(save);
		return 0;
	}
	
	frleinit_read(save);
	fhuffmaninit_read();
	
	// Check version number
	if(getc(save) != CROGUE_VERSION_MAJOR || getc(save) != CROGUE_VERSION_MINOR)
	{
		fclose(save);
		return 0;
	}
	
	frleread( (char*)(&w->m[0]), sizeof(monster)*MONSTERS_MAX, sizeof(monster), save );
	frleread( (char*)(map_size), sizeof(ushort)*2, sizeof(ushort), save );
	resize_map_store(map_size[0], map_size[1]);
	frleread( (char*)(&w->t[0][0]), sizeof(tile)*MAPSIZE_X*MAPSIZE_Y, sizeof(tile), save );
	
	place_player_on_map(source, pos);
	
	// Read items
	cleanup_items();
	frleread(&w->items, sizeof(itemsinfo), 1, save);
	w->items.items = (item*)debug_malloc(w->items.alloced*sizeof(item));
	frleread(w->items.items, sizeof(item)*w->items.alloced, sizeof(item), save);
	
	fclose(save);
	
#ifdef EXTERNAL_COMPRESSOR
	// Recompress file
	sprintf(cmd, EXTERNAL_COMPRESSOR " -f -q %s", name);
	system(cmd);
#endif

	full_redraw();
	
	return 1;
}
//}}}
//{{{
//
// Delete any remnants of saved games (the saved game and saved levels). This
// is called when the player dies to prevent him from using the files to cheat
// and bring his character back.
//
void cleanup_save(void)
{
	int i;
	
	unarchive_remove(savefile_name());
	
	for(i=1; i<=w->maxlevel+1; i++)
	{
		#ifdef EXTERNAL_COMPRESSOR
			unarchive_remove(levelfile_name_compressed(i));
		#else
			unarchive_remove(levelfile_name(i));
		#endif
	}
	
	cleanup_checkpoint();
}
//}}}
//{{{
static void cleanup_checkpoint(void)
{
	message("cleanup_checkpoint() called."); //DEBUG
	unarchive_remove(checkfile_name());
}
//}}}

//{{{
void compress_tiles(void)
{
	int i, j;
	
	for(i=0; i<MAPSIZE_Y; i++)
	for(j=0; j<MAPSIZE_X; j++)
	{
		w->t[i][j].flags &= ~TFLAG_LIT;
	}
}
//}}}

