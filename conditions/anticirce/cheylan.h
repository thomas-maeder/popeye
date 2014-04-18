#if !defined(CONDITIONS_ANTICIRCE_CHEYLAN_H)
#define CONDITIONS_ANTICIRCE_CHEYLAN_H

/* This module helps filtering out Anticirce rebirths that are not allowed
 * because of the Cheylan variety of Anticirce
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
void anticirce_cheylan_filter_solve(slice_index si);

/* Instrument a stipulation for Circe Cheylan
 * @param si identifies root slice of stipulation
 */
void anticirce_cheylan_initialise_solving(slice_index si);

#endif
