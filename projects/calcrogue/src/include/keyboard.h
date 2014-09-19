// keyboard.h
/// @file
/// @brief Keycodes and commands for various platforms

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "machdep.h" // FIXME: Workaround for prctools stupidity

#define KEY_NOP 0

#ifdef CPROTO
#	define SUPPORT_HOTKEY_ITEM
#	define KEY_SCROLLING
#endif
//{{{
#if TARGET == T_TI89
#define KEY_ACKNOWLEDGE              13
#define KEY_CONFIRM                  13
#define KEY_WEAR                     '('
#define KEY_DROP                     ')'
#define KEY_CLOSEDOOR                '+'
#define KEY_PICKUP                   ','
#define KEY_OPENDOOR                 '-'
#define KEY_THROW                    '/'
#define KEY_STAIR                    '0'
#define KEY_SOUTHWEST                '1'
#define KEY_SOUTH                    '2'
#define KEY_SOUTHEAST                '3'
#define KEY_WEST                     '4'
#define KEY_WAIT                     '5'
#define KEY_EAST                     '6'
#define KEY_NORTHWEST                '7'
#define KEY_NORTH                    '8'
#define KEY_NORTHEAST                '9'
#define KEY_WEAR_MULTIPLE            'B'
#define KEY_DROP_MULTIPLE            'C'
#define KEY_RUN_NORTHWEST            'G'
#define KEY_RUN_NORTH                'H'
#define KEY_RUN_NORTHEAST            'I'
#define KEY_RUN_WEST                 'L'
#define KEY_RUN_EAST                 'N'
#define KEY_RUN_SOUTHWEST            'Q'
#define KEY_RUN_SOUTH                'R'
#define KEY_RUN_SOUTHEAST            'S'
#define KEY_TAKEOFF_MULTIPLE         'T'
#define KEY_SEARCH                   '^'
#define KEY_TAKEOFF                  't'
#define KEY_FIRE                     'x'
#define KEY_CAST                     'y'
#define KEY_USE                      'z'
#define KEY_INVENTORY                '|'
#define KEY_REDRAW                   263
#define KEY_ESC                      264
#define KEY_ACT_MENU                 265
#define KEY_OPTIONS_MENU             266
#define KEY_SAVEGAME                 267
#define KEY_HELPMENU                 268
#define KEY_STATS                    269
#define KEY_ABOUT                    270
#define KEY_MESSAGE_HISTORY          271
#define KEY_DEBUG                    272
#define SUPPORT_HOTKEY_ITEM
#define KEY_HOTKEY_ITEM              277
#define KEY_DISCOVERIES              278
#define KEY_SCROLLING
#define KEY_SCROLL_SIZE              8
#define KEY_SCROLL_UP                337
#define KEY_SCROLL_LEFT              338
#define KEY_SCROLL_DOWN              340
#define KEY_SCROLL_RIGHT             344
#endif //T_TI89
//}}}
//{{{
#if TARGET == T_TI92P
#define KEY_ACKNOWLEDGE              13
#define KEY_CONFIRM                  13
#define KEY_WEAR                     '('
#define KEY_DROP                     ')'
#define KEY_CLOSEDOOR                '+'
#define KEY_PICKUP                   ','
#define KEY_OPENDOOR                 '-'
#define KEY_THROW                    '/'
#define KEY_STAIR                    '0'
#define KEY_SOUTHWEST                '1'
#define KEY_SOUTH                    '2'
#define KEY_SOUTHEAST                '3'
#define KEY_WEST                     '4'
#define KEY_WAIT                     '5'
#define KEY_EAST                     '6'
#define KEY_NORTHWEST                '7'
#define KEY_NORTH                    '8'
#define KEY_NORTHEAST                '9'
#define KEY_SEARCH                   '^'
#define KEY_INVENTORY                'i'
#define KEY_FIRE                     259
#define KEY_CAST                     260
#define KEY_USE                      261
#define KEY_TAKEOFF                  262
#define KEY_REDRAW                   263
#define KEY_ESC                      264
#define KEY_ACT_MENU                 265
#define KEY_OPTIONS_MENU             266
#define KEY_SAVEGAME                 267
#define KEY_HELPMENU                 268
#define KEY_STATS                    269
#define KEY_ABOUT                    270
#define KEY_DISCOVERIES              271
#define KEY_DEBUG                    272
#define SUPPORT_HOTKEY_ITEM
#define KEY_HOTKEY_ITEM              273
#define KEY_MESSAGE_HISTORY          274
#define KEY_SCROLLING
#define KEY_SCROLL_SIZE              8
#define KEY_SCROLL_LEFT              337
#define KEY_SCROLL_UP                338
#define KEY_SCROLL_RIGHT             340
#define KEY_SCROLL_DOWN              344
#define KEY_WEAR_MULTIPLE            8232
#define KEY_DROP_MULTIPLE            8233
#define KEY_RUN_SOUTHWEST            8241
#define KEY_RUN_SOUTH                8242
#define KEY_RUN_SOUTHEAST            8243
#define KEY_RUN_WEST                 8244
#define KEY_RUN_EAST                 8246
#define KEY_RUN_NORTHWEST            8247
#define KEY_RUN_NORTH                8248
#define KEY_RUN_NORTHEAST            8249
#define KEY_TAKEOFF_MULTIPLE         8454

#endif //T_TI92P
//}}}
//{{{
#ifdef USE_CURSES
#define KEY_SOUTHWEST                '1'
#define KEY_SOUTH                    '2'
#define KEY_SOUTHEAST                '3'
#define KEY_WEST                     '4'
#define KEY_WAIT                     '5'
#define KEY_EAST                     '6'
#define KEY_NORTHWEST                '7'
#define KEY_NORTH                    '8'
#define KEY_NORTHEAST                '9'

#ifdef UNIX
#define KEY_RUN_SOUTHWEST            360
#define KEY_RUN_SOUTH                258
#define KEY_RUN_SOUTHEAST            338
#define KEY_RUN_WEST                 260
#define KEY_RUN_EAST                 261
#define KEY_RUN_NORTHWEST            262
#define KEY_RUN_NORTH                259
#define KEY_RUN_NORTHEAST            339
#else //UNIX
#define KEY_RUN_SOUTHWEST            CTL_PAD1
#define KEY_RUN_SOUTH                CTL_PAD2
#define KEY_RUN_SOUTHEAST            CTL_PAD3
#define KEY_RUN_WEST                 CTL_PAD4
#define KEY_RUN_EAST                 CTL_PAD6
#define KEY_RUN_NORTHWEST            CTL_PAD7
#define KEY_RUN_NORTH                CTL_PAD8
#define KEY_RUN_NORTHEAST            CTL_PAD9
#endif //UNIX

#define KEY_OPTIONS_MENU             'O'

#define KEY_ACT_MENU                 'a'
#define KEY_STAIR                    '0'
#define KEY_INVENTORY                'i'
#define KEY_WEAR                     'w'
#define KEY_WEAR_MULTIPLE            'W'
#define KEY_TAKEOFF                  't'
#define KEY_TAKEOFF_MULTIPLE         'T'
#define KEY_DROP                     'd'
#define KEY_DROP_MULTIPLE            'D'
#define SUPPORT_HOTKEY_ITEM
#define KEY_HOTKEY_ITEM              'h'
#define KEY_PICKUP                   ','
#define KEY_CAST                     'Z'
#define KEY_USE                      'z'
#define KEY_THROW                    'v'
#define KEY_FIRE                     'f'
#define KEY_SEARCH                   's'
#define KEY_HELPMENU                 KEY_F(1)
#define KEY_STATS                    KEY_F(2)
#define KEY_ABOUT                    KEY_F(3)
#define KEY_DEBUG                    '_'
#define KEY_OPENDOOR                 'o'
#define KEY_CLOSEDOOR                'c'
#define KEY_ESC                      27
#define KEY_SAVEGAME                 'S'
#define KEY_ACKNOWLEDGE              '\n'
#define KEY_CONFIRM                  '\r'
#define KEY_MESSAGE_HISTORY          'H'
#define KEY_DISCOVERIES              '\\'
#define KEY_REDRAW                   'R'
#endif //USE_CURSES
//}}}
//{{{
#if TARGET == T_PALMOS

	// Some things are internally considered 'keys' but are actually pen
	// strokes or something else.
#define NOTAKEY                      5000

#define KEY_SOUTHWEST                'b'
#define KEY_SOUTH                    'j'
#define KEY_SOUTHEAST                'n'
#define KEY_WEST                     'h'
#define KEY_WAIT                     '.'
#define KEY_EAST                     'l'
#define KEY_NORTHWEST                'y'
#define KEY_NORTH                    'k'
#define KEY_NORTHEAST                'u'

#define KEY_RUN_SOUTHWEST            'B'
#define KEY_RUN_SOUTH                'J'
#define KEY_RUN_SOUTHEAST            'N'
#define KEY_RUN_WEST                 'H'
#define KEY_RUN_EAST                 'L'
#define KEY_RUN_NORTHWEST            'Y'
#define KEY_RUN_NORTH                'K'
#define KEY_RUN_NORTHEAST            'U'
#define KEY_RUN                      (NOTAKEY+17)

// On PalmOS only, there're special strokes for firing readied ammo
#define KEY_FIRING
#define KEY_FIRE_SOUTHWEST            (NOTAKEY)
#define KEY_FIRE_SOUTH                (NOTAKEY+1)
#define KEY_FIRE_SOUTHEAST            (NOTAKEY+2)
#define KEY_FIRE_WEST                 (NOTAKEY+3)
#define KEY_FIRE_EAST                 (NOTAKEY+4)
#define KEY_FIRE_NORTHWEST            (NOTAKEY+5)
#define KEY_FIRE_NORTH                (NOTAKEY+6)
#define KEY_FIRE_NORTHEAST            (NOTAKEY+7)
#define KEY_SCROLLING
#define KEY_SCROLLING_DIAGONAL
#define KEY_SCROLL_SIZE              8
#define KEY_SCROLL_DOWNLEFT             (NOTAKEY+8)
#define KEY_SCROLL_DOWN                 (NOTAKEY+9)
#define KEY_SCROLL_DOWNRIGHT            (NOTAKEY+10)
#define KEY_SCROLL_LEFT                 (NOTAKEY+11)
#define KEY_SCROLL_RIGHT                (NOTAKEY+12)
#define KEY_SCROLL_UPLEFT               (NOTAKEY+13)
#define KEY_SCROLL_UP                   (NOTAKEY+14)
#define KEY_SCROLL_UPRIGHT              (NOTAKEY+15)
#define KEY_SCROLL_PROMPT               (NOTAKEY+16)

#define KEY_OPTIONS_MENU             'O'

#define KEY_ACT_MENU                 'a'
#define KEY_STAIR                    '0'
#define KEY_INVENTORY                'i'
#define KEY_WEAR                     'w'
#define KEY_WEAR_MULTIPLE            'W'
#define KEY_TAKEOFF                  't'
#define KEY_TAKEOFF_MULTIPLE         'T'
#define KEY_DROP                     'd'
#define KEY_DROP_MULTIPLE            'D'
#define KEY_PICKUP                   ','
#define KEY_CAST                     'Z'
#define KEY_USE                      'z'
#define KEY_THROW                    'v'
#define KEY_FIRE                     'f'
#define KEY_SEARCH                   's'
#define KEY_HELPMENU                 '?'
#define KEY_STATS                    '@'
#define KEY_ABOUT                    'V'
#define KEY_DEBUG                    '_'
#define KEY_OPENDOOR                 'o'
#define KEY_CLOSEDOOR                'c'
#define KEY_ESC                      'Q'
#define KEY_SAVEGAME                 'S'
#define KEY_ACKNOWLEDGE              '\n'
#define KEY_CONFIRM                  '\r'
#define KEY_MESSAGE_HISTORY          'P'
#define KEY_DISCOVERIES              '\\'
#define KEY_REDRAW                   'R'
#endif //PALMOS
//}}}

#endif //KEYBOARD_H
