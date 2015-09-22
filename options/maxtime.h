#if !defined(OPTIONS_MAXTIME_H)
#define OPTIONS_MAXTIME_H

/* This module provides functionality dealing with the filter slices
 * implement the maxtime option and command line parameter.
 * Slices of this type make sure that solving stops after the maximum
 * time has elapsed
 */

#include "solving/machinery/solve.h"
#include "platform/maxtime.h"

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
void maxtime_guard_solve(slice_index si);

/* Allocate a STMaxTimeGuard slice.
 * @param incomplete identifies the slice to propagage information about
 *                   interruption to
 * @return allocated slice
 */
slice_index alloc_maxtime_guard(slice_index incomplete);

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
void maxtime_set(slice_index si);

/* Instrument the solving machinery
 * @param si identifies the slice where to start instrumenting
 */
void maxtime_problem_instrumenter_solve(slice_index si);

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 * @param maxtime
 */
void maxtime_instrument_solving(slice_index si, maxtime_type maxtime);

#endif
