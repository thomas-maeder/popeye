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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type castling_remove_non_reachers_solve(slice_index si,
                                                    stip_length_type n);

#endif
