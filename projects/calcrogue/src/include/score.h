// score.h
/// @file
/// @brief High-score keeping
/// @sa score.c

#ifndef SCORE_H
#define SCORE_H

#define CAUSE_OF_DEATH_MAX_LENGTH 32

/// A high score
typedef struct high_score
{
	ulong points;                                   ///< The final score
	char cause_of_death[CAUSE_OF_DEATH_MAX_LENGTH]; ///< How it ended
} high_score;

/// A high-score file
typedef struct high_scores
{
	high_score scores[NUM_SCORES];
	ulong num_games;
	ulong checksum;      ///< Generated with #checksum_scores
} high_scores;

#endif //SCORE_H

