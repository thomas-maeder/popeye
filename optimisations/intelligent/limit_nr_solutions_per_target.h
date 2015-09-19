#if !defined(OPTIMISATION_INTELLIGENT_LIMIT_NR_SOLUTIONS_PER_TARGET_POS_H)
#define OPTIMISATION_INTELLIGENT_LIMIT_NR_SOLUTIONS_PER_TARGET_POS_H

#include "solving/machinery/solve.h"

/* This module provides the slice types that limit the number of solutions per
 * target position if requested by the user
 */

/* Propagate our findings to STProblemSolvingInterrupted
 * @param si identifies the slice where to start instrumenting
 */
void intelligent_nr_solutions_per_target_position_solving_instrumenter_solve(slice_index si);

/* Instrument the solving machinery with option intelligent n
 * @param si identifies the slice where to start instrumenting
 * @param max_nr_solutions_per_target_position
 */
void intelligent_nr_solutions_per_target_position_instrument_solving(slice_index si,
                                                                     unsigned long max_nr_solutions_per_target_position);

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
void intelligent_nr_solutions_per_target_position_counter_solve(slice_index si);

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
void intelligent_limit_nr_solutions_per_target_position_solve(slice_index si);

#endif
