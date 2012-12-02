#if !defined(OPTIMISATION_GOALS_CASTLING_REMOVE_NON_REACHERS_H)
#define OPTIMISATION_GOALS_CASTLING_REMOVE_NON_REACHERS_H

#include "solving/solve.h"

/* This module provides slice type STCastlingRemoveNonReachers.
 * Slices of this type optimise by removing moves that are not en passant
 * captures in situations where an en passant capture is required.
 */

/* Allocate a STCastlingRemoveNonReachers slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_remove_non_reachers_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type castling_remove_non_reachers_solve(slice_index si,
                                                    stip_length_type n);

#endif
