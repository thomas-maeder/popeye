#if !defined(OPTIMISATIONS_INTELLIGENT_MOVES_LEFT_H)
#define OPTIMISATIONS_INTELLIGENT_MOVES_LEFT_H

#include "stipulation/help_play/play.h"

extern unsigned int MovesLeft[nr_sides];

/* This module provides functionality dealing with STIntelligentFilter
 * stipulation slice type.
 * Slices of this type make solve help stipulations in intelligent mode
 */

/* Allocate a STIntelligentMovesLeftInitialiser slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_moves_left_initialiser(void);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type intelligent_moves_left_initialiser_help(slice_index si,
                                                         stip_length_type n);

#endif
