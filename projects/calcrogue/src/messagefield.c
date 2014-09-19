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
// messagefield.c
/// @file
/// @brief Shows messages at the top of the screen

#include "crogue.h"
#include "export.h"

#define MESSAGEFIELD_X 0
#define MESSAGEFIELD_Y 0
#define MESSAGEFIELD_ROWS messagefield_rows
#define MESSAGEFIELD_HEIGHT messagefield_height
#define MESSAGEFIELD_WIDTH SCREEN_WIDTH

static short messagefield_y=0, messagefield_rows=2, messagefield_height=16;

static void _UI_MF_savemsg(const char *msg);
static draw_string_info MF_state;

//{{{
void UI_MF_Place(short ypos, short rows)
{
	messagefield_y = ypos;
	messagefield_rows = rows;
	messagefield_height = rows*FontHeight();
}
//}}}

//{{{
/// @brief Show text to the player
///
/// This function works like printf, except that text goes to the message
/// line. See documentation for 'printf' on how to make format strings.
/// If the message is too long, it will automatically be wrapped and
/// the player will be given a -More- prompt as needed.
/// @sa vmessage
void message(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vmessage(format, args);
}
//}}}
//{{{
/// @brief Show text to the player with a va_list
///
/// This function works like vprintf, meaning it takes its arguments in the
/// form of a va_list and otherwise acts like printf.
/// @sa message
void vmessage(const char *format, va_list args)
{
	char buf[512];
	
	w->interrupt = 1;
	
	vsprintf(buf, format, args);
	
	if(!w->messagevis)
		UI_MF_clear();

	_UI_MF_savemsg(buf);
	UI_MF_sendmsg(buf);
	w->messagevis = 1;
}
//}}}

//{{{
/// @brief Clear the message line
void UI_MF_clear(void)
{
#ifdef USE_CURSES
	int i;
#endif
	// Reset pos
	MF_state.curX = 0;
	MF_state.curRow = 0;
	
#ifdef IS_CALCULATOR
	SetFont(get_option(OPTION_FONTSIZE));
	clear_string(0, 0, MESSAGEFIELD_WIDTH, MESSAGEFIELD_ROWS);
#endif
#ifdef USE_CURSES
	for(i=0; i<MESSAGEFIELD_ROWS; i++)
		clear_line(i);
#endif
#ifdef PALMOS
	Graph_ClearRect(MESSAGEFIELD_X, MESSAGEFIELD_Y,
	                MESSAGEFIELD_WIDTH, MESSAGEFIELD_HEIGHT-1);
#endif
}
//}}}

static char message_buf[MESSAGEBUF_SIZE];

//{{{
/// @brief Stores a string to the message history
static void _UI_MF_savemsg(const char *msg)
{
	int i;
	
	// Make room for new message
	while(strlen(message_buf) + strlen(msg) + 2 > MESSAGEBUF_SIZE)
	{
		// Find start of next string
		for(i=0; i<strlen(message_buf); i++)
			if(message_buf[i] == '\n')
				break;
		strcpy(message_buf, message_buf+i+1);
	}
	
	strcat(message_buf, msg);
	strcat(message_buf, "\n");
}
//}}}
//{{{
/// @brief Display the message history
void UI_MF_history(void)
{
	char converted_buf[MESSAGEBUF_SIZE]; // For converting newlines to spaces
	draw_string_info state = {0, 0};
	int i;
	strcpy(converted_buf, message_buf);
	
	for(i=0; i<MESSAGEBUF_SIZE; i++)
		if(converted_buf[i] == '\n') converted_buf[i] = ' ';
	
	Graph_ClrScr();
#ifdef IS_CALCULATOR
	SetFont(get_option(OPTION_FONTSIZE));
#endif
	draw_string(converted_buf, &state, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	read_char();
	full_redraw();
}
//}}}
//{{{
/// @brief Send a message string
///
/// This function is used internally by message; it shows a string in the
/// status line, wrapping and -More-'ing as needed. Note, however, that it
/// does not support format specifiers or extra arguments like printf and
/// message do. This function should not be called directly - instead,
/// call #message.
///
/// @sa message
void UI_MF_sendmsg(const char *msg)
{
#ifdef IS_CALCULATOR
	SetFont(get_option(OPTION_FONTSIZE));
#endif
	
	if(MF_state.curX)
		draw_string(" ", &MF_state, MESSAGEFIELD_X, MESSAGEFIELD_Y, MESSAGEFIELD_WIDTH, MESSAGEFIELD_HEIGHT, 1);
	
	draw_string(msg, &MF_state, MESSAGEFIELD_X, MESSAGEFIELD_Y, MESSAGEFIELD_WIDTH, MESSAGEFIELD_HEIGHT, 1);
	
#ifdef CURSES
	refresh();
#endif
}
//}}}
//{{{
/// @brief Give a -More- prompt
///
/// Displays "-More-", then waits for the user to press a key and clears the
/// message line.
void UI_MF_wait(void)
{
	UI_MF_sendmsg(gettext("-More-"));
	read_char();
	UI_MF_clear();
}
//}}}

