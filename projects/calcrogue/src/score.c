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
// score.c
/// @file
/// @brief High scores

#include "crogue.h"
#include "export.h"

static ulong checksum_scores(high_scores *scores);

static const uchar scoreScreenNumTabStops = 3;
#ifdef IS_CALCULATOR
#	define SCORESCREEN_LEFT 2
#	define SCORESCREEN_HEADER_LEFT 40
#	define SCORESCREEN_ARROW_SPACE 18
#	define SCORESCREEN_SCORE_SPACE 45
#endif
#ifdef REALCOMPUTER
#	define SCORESCREEN_LEFT 11
#	define SCORESCREEN_HEADER_LEFT 30
#	define SCORESCREEN_ARROW_SPACE 5
#	define SCORESCREEN_SCORE_SPACE 13
#endif
#ifdef PALMOS
#	define SCORESCREEN_LEFT 2
#	define SCORESCREEN_HEADER_LEFT 40
#	define SCORESCREEN_ARROW_SPACE 20
#	define SCORESCREEN_SCORE_SPACE 45
#endif

//{{{
void display_scores(ulong score)
{
	FILE *scorefile;
	high_scores table;
	int i;
	int score_highlighted = 0;
	char buf[64];
	draw_string_info state = {0, 0};
	
#ifdef REALCOMPUTER
	char full_filename[256];
	snprintf(full_filename, 256, "%s" SCOREFILENAME, file_prefix);
	scorefile = fopen(full_filename, "rb");
#else
	scorefile = fopen(SCOREFILENAME, "rb");
#endif
	if(!scorefile)
		return;
	if(fread(&table, sizeof(high_scores), 1, scorefile)<1)
		return;
	fclose(scorefile);
	
	Graph_ClrScr();
#ifdef IS_CALCULATOR
	SetFont(OPTION_FONT_SMALL);
#endif
	setTabStops(3, 0, SCORESCREEN_ARROW_SPACE, SCORESCREEN_SCORE_SPACE);
	draw_string(gettext("\nHIGH SCORES\n\n"), &state, SCORESCREEN_HEADER_LEFT, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	for(i=0; i < min(table.num_games, NUM_SCORES); i++)
	{
		if(table.scores[i].points==score && !score_highlighted)
		{
			score_highlighted=1;
			strcpy(buf, "--->");
		} else {
			strcpy(buf, "");
		}
		catprintf(buf, "\t%li\t%s\n", table.scores[i].points, table.scores[i].cause_of_death);

		draw_string(buf, &state, SCORESCREEN_LEFT, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	}
	for(; i < NUM_SCORES; i++)
		draw_string("\n", &state, SCORESCREEN_LEFT, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	
	sprintf(buf, gettext("\n%li games played"), table.num_games);
	if(score != -1)
		catprintf(buf, gettext("\nYour score was %li"), score);
	draw_string(buf, &state, SCORESCREEN_LEFT+SCORESCREEN_ARROW_SPACE, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	
	read_char();
}
//}}}
//{{{
void award_high_score(ulong score, const char *cause_of_death)
{
	FILE *scorefile;
	high_scores table;
	int pos, i;
#ifdef REALCOMPUTER
	char full_filename[256];
#endif
	memset(&table, 0, sizeof(table));
	
	// Read old scores
#ifdef REALCOMPUTER
	snprintf(full_filename, 256, "%s" SCOREFILENAME, file_prefix);
	scorefile = fopen(full_filename, "rb");
#else
	scorefile = fopen(SCOREFILENAME, "rb");
#endif
	if(scorefile)
	{
		fread(&table, sizeof(high_scores), 1, scorefile);
		fclose(scorefile);
		
		if(table.checksum != checksum_scores(&table))
		{
			message(gettext("My scores file has been tampered with...\n"
					"Did you think I wouldn't notice?"));
			read_char();
			remove(SCOREFILENAME);
			return;
		}
	}
	
	// Add a game
	table.num_games++;
	if(table.num_games==1)
	{
		table.scores[0].points = score;
#		ifdef SHARED_SCORES
			snprintf(table.scores[0].cause_of_death, CAUSE_OF_DEATH_MAX_LENGTH,
			         "%s. %s", getlogin(), cause_of_death);
#		else
			strcpy(table.scores[0].cause_of_death, cause_of_death);
#		endif
	} else {
		for(pos=0; pos<NUM_SCORES; pos++)
		{
			if(table.scores[pos].points <= score)
			{
				for(i=NUM_SCORES-1; i>pos; i--)
					table.scores[i] = table.scores[i-1];
				table.scores[pos].points = score;
#				ifdef SHARED_SCORES
					sprintf(table.scores[pos].cause_of_death, "%s. %s", getlogin(), cause_of_death);
#				else
					strcpy(table.scores[pos].cause_of_death, cause_of_death);
#				endif
				break;
			}
		}
	}
	table.checksum = checksum_scores(&table);
	
#	ifdef IS_CALCULATOR
	EM_moveSymFromExtMem(SYMSTR_CONST(SCOREFILENAME), HS_NULL);
#	endif
	
	// Write the scores
#ifdef REALCOMPUTER
	scorefile = fopen(full_filename, "wb");
#else
	scorefile = fopen(SCOREFILENAME, "wb");
#endif
	if(scorefile)
	{
		fwrite(&table, sizeof(high_scores), 1, scorefile);
		fclose(scorefile);
	} else {
		message(gettext("Error writing score file."));
	}
#ifdef IS_CALCULATOR
	if(get_option(OPTION_ARCHIVESCORES) == OPTION_ARCHIVE_YES)
		archive_file(SCOREFILENAME, sizeof(scorefile));
#endif
}
//}}}
//{{{
static ulong checksum_scores(high_scores *scores)
{
	return checksum_bytes(scores, sizeof(*scores) - sizeof(scores->checksum));
}
//}}}

