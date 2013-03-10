#if !defined(OPTIMISATION_GOALS_TARGET_REMOVE_NON_REACHERS_H)
#define OPTIMISATION_GOALS_TARGET_REMOVE_NON_REACHERS_H

#include "solving/solve.h"

/* This module provides slice type STTargetRemoveNonReachers.
 * Slices of this type optimise by removing moves that are not moves reaching
 * that goal.
 */

/* Allocate a STTargetRemoveNonReachers slice.
 * @param target target square to be reached
 * @return index of allocated slice
 */
slice_index alloc_target_remove_non_reachers_slice(square target);

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
stip_length_type target_remove_non_reachers_solve(slice_index si,
                                                  stip_length_type n);

#endif
