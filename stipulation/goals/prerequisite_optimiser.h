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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
goal_prerequisite_optimiser_solve(slice_index si, stip_length_type n);

#endif
