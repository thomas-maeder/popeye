#if !defined(SOLVING_DEAD_END_H)
#define SOLVING_DEAD_END_H

/* This module provides functionality dealing with the defending side
 * in STDeadEnd stipulation slices.
 */

#include "solving/machinery/solve.h"

/* Optimise away redundant deadend slices
 * @param si identifies the entry slice
 */
void solving_optimise_dead_end_slices(slice_index si);

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
void dead_end_solve(slice_index si);

#endif
