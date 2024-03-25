#if !defined(CONDITIONS_FUDDLEDMEN_H)
#define CONDITIONS_FUDDLEDMEN_H

/* This module implements the condition Fuddled Men */

#include "solving/machinery/solve.h"
#include "position/pieceid.h"

extern PieceIdType fuddled[nr_sides];

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
void fuddled_men_bookkeeper_solve(slice_index si);

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
void fuddled_men_generate_moves_for_piece(slice_index si);

/* Validate an observation according to Fuddled Men
 * @return true iff the observation is valid
 */
boolean fuddled_men_inverse_validate_observation(slice_index si);

/* Instrument the solving machinery for Fuddled Men
 * @param si identifies root slice of stipulation
 */
void fuddled_men_initialise_solving(slice_index si);

#endif
