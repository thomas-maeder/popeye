#if !defined(SOLVING_PLAY_SELFCHECK_GUARD_H)
#define SOLVING_PLAY_SELFCHECK_GUARD_H

/* STSelfCheckGuard slice - stops solutions with moves that expose the
 * own king
 */

#include "solving/machinery/solve.h"

/* Instrument the solving machinery with slices dealing with selfcheck detection
 * @param si identifies the root of the solving machinery
 */
void solving_insert_selfcheck_guards(slice_index si);

/* Allocate a STSelfCheckGuard slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_slice(void);

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
void selfcheck_guard_solve(slice_index si);

#endif
