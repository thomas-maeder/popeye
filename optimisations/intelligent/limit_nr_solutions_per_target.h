#if !defined(OPTIMISATION_INTELLIGENT_LIMIT_NR_SOLUTIONS_PER_TARGET_POS_H)
#define OPTIMISATION_INTELLIGENT_LIMIT_NR_SOLUTIONS_PER_TARGET_POS_H

#include "solving/machinery/solve.h"

/* This module provides the slice types that limit the number of solutions per
 * target position if requested by the user
 */

/* Reset the number of solutions per target position
 */
void reset_nr_solutions_per_target_position(void);

/* Reset status whether solving the current problem was affected because the limit
 * on the number of solutions per target position was reached.
 */
void intelligent_nr_solutions_per_target_position_restter_solve(slice_index si);

/* Determine whether solving the current problem was affected because the limit
 * on the number of solutions per target position was reached.
 * @return true iff solving was affected
 */
boolean was_max_nr_solutions_per_target_position_reached(void);

/* Attempt to read the maximum number of solutions per target position
 * @param tok next input token
 * @return true iff the maximum number could be read from tok
 */
boolean read_max_nr_solutions_per_target_position(char const *tok);

/* Determine whether the maximum number of solutions per target position is
 * limited
 * @return true iff the maximum number is limited
 */
boolean is_max_nr_solutions_per_target_position_limited(void);

/* Allocate a STIntelligentSolutionsPerTargetPosCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_nr_solutions_per_target_position_counter_slice(void);

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

/* Allocate a STIntelligentLimitNrSolutionsPerTargetPos slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_limit_nr_solutions_per_target_position_slice(void);

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
