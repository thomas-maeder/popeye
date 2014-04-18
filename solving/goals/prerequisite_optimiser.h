#if !defined(SOLVING_GOALS_PREREQUISITE_OPTIMISER_H)
#define SOLVING_GOALS_PREREQUISITE_OPTIMISER_H

#include "solving/machinery/solve.h"

/* This module provides functionality dealing with the defending side
 * in STPrerequisiteOptimiser stipulation slices.
 * Optimise solving in cases where all goals to be reached have prerequisites
 * and none of them are met.
 */

/* Allocate a STPrerequisiteOptimiser defender slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_prerequisite_optimiser_slice(void);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void goal_prerequisite_optimiser_solve(slice_index si);

#endif
