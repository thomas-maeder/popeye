#if !defined(CONDITIONS_DARKSIDE_H)
#define CONDITIONS_DARKSIDE_H

#include "position/side.h"
#include "solving/machinery/solve.h"

/* This module implements the condition Darkside */

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
void darkside_side_changer_solve(slice_index si);

/* Instrument slices with Darkside
 */
void solving_insert_darkside(slice_index si);

#endif
