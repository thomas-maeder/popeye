#if !defined(SOLVING_GOAL_ANY_REACHED_TESTER_H)
#define SOLVING_GOAL_ANY_REACHED_TESTER_H

#include "solving/machinery/solve.h"

/* This module provides functionality dealing with slices that detect
 * whether an any goal has just been reached
 */

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
void goal_any_reached_tester_solve(slice_index si);

#endif
