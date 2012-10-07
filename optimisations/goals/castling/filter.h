#if !defined(OPTIMISATIONS_GOALS_CASTLING_FILTER_H)
#define OPTIMISATIONS_GOALS_CASTLING_FILTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STCastlingFilter stipulation slices.
 * Slices of this type optimise solving the goal "castling" by
 * testing whether >=1 castling is legal in the final move.
 */

/* Allocate a STCastlingFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_filter_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type castling_filter_solve(slice_index si, stip_length_type n);

#endif
