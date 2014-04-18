#if !defined(CONDITIONS_CIRCE_CAPTURE_FORK_H)
#define CONDITIONS_CIRCE_CAPTURE_FORK_H

/* Bypass the Circe rebirth machinery if there is no capture */

#include "solving/machinery/solve.h"

/* Instrument the solving machinery with Circe Super (apart from the rebirth
 * square determination, whose instrumentation is elsewhere)
 * @param si identifies entry slice into solving machinery
 * @param variant identifies address of structure holding the Circe variant
 * @param interval_start type of slice that starts the sequence of slices
 *                       implementing that variant
 */
void circe_solving_instrument_nocapture_bypass(slice_index si,
                                               slice_type interval_start);

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
void circe_capture_fork_solve(slice_index si);

#endif
