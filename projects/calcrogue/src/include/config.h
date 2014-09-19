// config.h
/// @file
/// @brief Configuration options

#ifndef CONFIG_H
#define CONFIG_H

#define REGISTERVAR
                            ///< Keep the address of the game's state in a
                            /// register (for size improvement). Automatically
                            /// disabled on non-supporting platforms. Disable
                            /// this if your version of gcc is buggy.
#define SET_FILE_IN_USE_BIT
                            ///< Work around AMS twin-symbol bug
#define ALLOWDEBUG
                            ///< Allow use of the debug menu
#define DEBUG_HEAVILY
                            ///< Include expensive debugging code/features
#undef  INTERPRETER_DEBUG
                            ///< Include extra checks in the interpreter
#undef  DEBUG_BALANCE
                            ///< Display messages to indicate balance
#undef  DEBUG_ALLOCATOR
                            ///< Write guards around memory blocks
#undef  DEBUG_FILES
                            ///< Checksums on individual blocks in saves
#undef  DISABLE_COMPRESSION
                            ///< Disable compression in saves and data
#undef  DISABLE_HUFFMAN
                            ///< Disable Huffman compression, still use RLE
#undef  DISABLE_RLE
                            ///< Disable RLE compression, still use Huffman

#define CROGUE_VERSION "Beta 6c"
                                  ///< Version string
#define CROGUE_VERSION_MAJOR 6
                                  ///< Version number (major)
#define CROGUE_VERSION_MINOR 3
                                  ///< Version number (minor)

#define SAVEFILENAME "rgsave"
                                  ///< Name of save file
#define CHECKFILENAME "rgcheck"
                                  ///< Name of checkpoint save file
#define DATAFILENAME "crogdat"
                                  ///< (Calculator) name of data file
#define DATAFILENAME_PC "crogdat.dat"
                                  ///< (PC) Name of data file
#define SCOREFILENAME "rgscores"
                                  ///< Name of high-score file
#define OPTIONSFILENAME "rgopts"
                                  ///< Name of configuration file
#define NUM_SCORES 10
                                  ///< Maximum entries in high-score file
#define DLLFILENAME "crogdll"
                                  ///< (Calculator) name of DLL

    /// Number used to make sure it's the right DLL.
    /// Arbitrary, but should be different in each version.
#define DLL_IDENTIFICATION_NUMBER (0x6CA4F134+CROGUE_VERSION_MAJOR*10+CROGUE_VERSION_MINOR)

    /// Maximum # of monsters on one level at one time
#define MONSTERS_MAX 128 
    /// Amount of storage used for message history. Should be a power of 2.
#define MESSAGEBUF_SIZE 512 
    /// Number of items the player can carry at a time.
#define PLRINVSIZE 50

    /// Length of the delay() function on calculators, in timer ticks (1/20 s).
#define DELAY_AMOUNT 3
    /// Length of the delay() function on PC, in ms.
#define DELAY_AMOUNT_PC 80
    /// Multiplier to points for reaching a level
#define LEVEL_SCORE_FACTOR 100	
    /// Color used for messages, status line, etc
#define DEFAULT_COLOR 8

#endif //CONFIG_H

