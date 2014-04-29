#if !defined(CONDITIONS_EINSTEIN_EINSTEIN_H)
#define CONDITIONS_EINSTEIN_EINSTEIN_H

#include "pieces/pieces.h"
#include "solving/machinery/solve.h"

/* This module implements the condition Einstein Chess */

/* Decrease the rank of a piece
 * @param p piece whose rank to decrease
 */
piece_walk_type einstein_decrease_walk(piece_walk_type p);

/* Increase the rank of a piece
 * @param p piece whose rank to increase
 */
piece_walk_type einstein_increase_walk(piece_walk_type p);

/* Determine the capturer of the current move (if any)
 * @return departure square of the capturer; initsquare if the current move
 *                   isn't a capture
 */
square einstein_collect_capturers(void);

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
void einstein_moving_adjuster_solve(slice_index si);

/* Instrument slices with move tracers
 */
void solving_insert_einstein_moving_adjusters(slice_index si);

#endif
