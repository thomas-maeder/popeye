#if !defined(CONDITIONS_ANTICIRCE_TARGET_SQUARE_FILTER_H)
#define CONDITIONS_ANTICIRCE_TARGET_SQUARE_FILTER_H

#include "solving/solve.h"

/* This module provides slice type STAnticirceTargetSquareFilter - tests
 * additional conditions on some goals imposed by Anticirce:
 * - the mated side must have >=1 move that only fails because of self-check
 */

/* Allocate a STAnticirceTargetSquareFilter slice.
 * @param target target square to be reached
 * @return index of allocated slice
 */
slice_index alloc_anticirce_target_square_filter_slice(square target);

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
stip_length_type anticirce_target_square_filter_solve(slice_index si, stip_length_type n);

#endif
