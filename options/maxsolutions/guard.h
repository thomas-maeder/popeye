#if !defined(OPTIONS_MAXSOLUTIONS_GUARD_H)
#define OPTIONS_MAXSOLUTIONS_GUARD_H

#include "solving/machinery/solve.h"

/* This module provides functionality dealing with
 * STMaxSolutionsGuard stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsCounter slice.
 * @param interruption identifies interruption slice
 * @return allocated slice
 */
slice_index alloc_maxsolutions_counter_slice(slice_index interruption);

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
void maxsolutions_counter_solve(slice_index si);

/* Allocate a STMaxSolutionsGuard slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_guard_slice(void);

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
void maxsolutions_guard_solve(slice_index si);

#endif
