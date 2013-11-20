#if !defined(STIPULATION_GOALS_PREREQUISITE_OPTIMISER_H)
#define STIPULATION_GOALS_PREREQUISITE_OPTIMISER_H

#include "solving/solve.h"

/* This module provides functionality dealing with the defending side
 * in STPrerequisiteOptimiser stipulation slices.
 * Optimise solving in cases where all goals to be reached have prerequisites
 * and none of them are met.
 */

/* Allocate a STPrerequisiteOptimiser defender slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_prerequisite_optimiser_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type
goal_prerequisite_optimiser_solve(slice_index si, stip_length_type n);

#endif
