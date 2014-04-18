#if !defined(CONDITIONS_GENEVA_H)
#define CONDITIONS_GENEVA_H

#include "stipulation/stipulation.h"
#include "conditions/circe/circe.h"
#include "utilities/boolean.h"

/* This module implements the condition Geneva Chess */

extern circe_variant_type geneva_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void geneva_reset_variant(circe_variant_type *variant);

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
void geneva_initialise_reborn_from_capturer_solve(slice_index si);

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
void geneva_stop_catpure_from_rebirth_square_solve(slice_index si);

/* Initialise solving in Geneva Chess
 * @param si identifies the root slice of the stipulation
 */
void geneva_initialise_solving(slice_index si);

#endif
