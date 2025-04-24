#if !defined(CONDITIONS_MARSCIRCE_ANTI_H)
#define CONDITIONS_MARSCIRCE_ANTI_H

#include "solving/machinery/solve.h"
#include "conditions/circe/circe.h"

/* This module provides implements the condition Anti-Mars-Circe
 */

extern circe_variant_type antimars_variant;
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
void anti_marscirce_move_castling_partner_to_rebirth_square_solve(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param index index of this generator slice
 */
void anti_mars_circe_second_rebirth_for_castling(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param index index of this generator slice
 */
void anti_mars_circe_only_castling_after_second_rebirth(slice_index si);

/* Prevent Anti-Marscirce from generating null moves
 * @param si identifies the slice
 */
void anti_mars_circe_reject_null_moves(slice_index si);

/* Inialise solving in Anti-Mars Circe
 */
void solving_initialise_antimars(slice_index si);

#endif
