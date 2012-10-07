#if !defined(OPTIMISATION_INTELLIGENT_LIMIT_NR_SOLUTIONS_PER_TARGET_POS_H)
#define OPTIMISATION_INTELLIGENT_LIMIT_NR_SOLUTIONS_PER_TARGET_POS_H

#include "solving/solve.h"

/* This module provides the slice types that limit the number of solutions per
 * target position if requested by the user
 */

/* Reset the maximum number of solutions per target position
 */
void reset_max_nr_solutions_per_target_position(void);

/* Reset the number of solutions per target position
 */
void reset_nr_solutions_per_target_position(void);

/* Reset status whether solving the current problem was affected because the limit
 * on the number of solutions per target position was reached.
 */
void reset_was_max_nr_solutions_per_target_position_reached(void);

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
intelligent_nr_solutions_per_target_position_counter_solve(slice_index si,
                                                            stip_length_type n);

/* Allocate a STIntelligentLimitNrSolutionsPerTargetPos slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_limit_nr_solutions_per_target_position_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
intelligent_limit_nr_solutions_per_target_position_solve(slice_index si,
                                                          stip_length_type n);

#endif
