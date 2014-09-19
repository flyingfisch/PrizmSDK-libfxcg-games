// options.h
/// @file
/// @brief In-game configurable options

#ifndef OPTIONS_H
#define OPTIONS_H

#define OPTIONS_BASE 3

#ifdef SUPPORT_COLOR
#	define OPTIONS_COLOR 1
#else
#	define OPTIONS_COLOR 0
#endif
#ifdef IS_CALCULATOR
#	define OPTIONS_CALC 5
#else
#	define OPTIONS_CALC 0
#endif
#ifdef IS_PALM
#	define OPTIONS_PALM 2
#else
#	define OPTIONS_PALM 0
#endif

#define NUM_OPTIONS (OPTIONS_BASE+OPTIONS_COLOR+OPTIONS_CALC+OPTIONS_PALM)

#define OPTION_YES    0
#define OPTION_NO     1
#define OPTION_PROMPT 2

#define OPTION_AUTOPICKUP     0x0000
#define	OPTION_AUTO_YES             0
#define	OPTION_AUTO_NO              1
#define	OPTION_AUTO_PROMPT          2
#define OPTION_AUTOFOLLOW     0x0001

#define OPTION_COLOR          0x0002
#define OPTION_PROMPT_DLG     0x0003
#define OPTION_INVERSE        0x0004

#define OPTION_TILESIZE       0x0005
#define OPTION_SIZE_SMALL           0
#define OPTION_SIZE_LARGE           1
#define OPTION_FONTSIZE       0x0006
#define OPTION_FONT_SMALL           0
#define OPTION_FONT_MEDIUM          1
#define OPTION_FONT_LARGE           2
#define OPTION_AUTOARCHIVE    0x0007
#define OPTION_ARCHIVESCORES  0x0008
#define OPTION_ARCHIVEOPTIONS 0x0009
#define OPTION_ARCHIVE_YES          0
#define OPTION_ARCHIVE_NO           1
#define OPTION_GRAYSCALE      0x000A
#define OPTION_GRAY_ON              0
#define OPTION_GRAY_OFF             1
#define OPTION_CHECKPOINT     0x000B

#endif

