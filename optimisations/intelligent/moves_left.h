#if !defined(OPTIMISATIONS_INTELLIGENT_MOVES_LEFT_H)
#define OPTIMISATIONS_INTELLIGENT_MOVES_LEFT_H

#include "solving/solve.h"

extern unsigned int MovesLeft[nr_sides];

/* This module provides functionality dealing with STIntelligentMovesLeftInitialiser
 * stipulation slice type.
 * Slices of this type make solve help stipulations in intelligent mode
 */

/* Allocate a STIntelligentMovesLeftInitialiser slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_moves_left_initialiser(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type intelligent_moves_left_initialiser_solve(slice_index si,
                                                           stip_length_type n);

#endif
