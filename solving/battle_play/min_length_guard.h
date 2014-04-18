#if !defined(SOLVING_BATTLE_PLAY_MIN_LENGTH_H)
#define SOLVING_BATTLE_PLAY_MIN_LENGTH_H

/* make sure that the minimum length of a branch is respected
 */

#include "solving/machinery/solve.h"

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
void min_length_guard_solve(slice_index si);

/* Instrument the stipulation with minimum length functionality
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_min_length(slice_index si);

#endif
