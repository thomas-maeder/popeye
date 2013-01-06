#if !defined(SOLVING_RECURSION_STOPPER_H)
#define SOLVING_RECURSION_STOPPER_H

/* This module implements the slice type STRecursionStopper which can be used
 * to avoid infinite recursion in chess forms that allow for it
 */

#include "solving/solve.h"

/* Allocate a STRecursionStopper slice
 * @return newly allocated slice
 */
slice_index alloc_recursion_stopper_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type recursion_stopper_solve(slice_index si, stip_length_type n);

#endif
