#if !defined(CONDITIONS_CIRCE_REX_INCLUSIVE_H)
#define CONDITIONS_CIRCE_REX_INCLUSIVE_H

/* This module implements Circe rex inclusive */

#include "stipulation/stipulation.h"
#include "conditions/circe/circe.h"
#include "utilities/boolean.h"

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
void circe_prevent_king_rebirth_solve(slice_index si);

/* Inialise solving in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start slice type starting the sequence of slices for the variant
 */
void circe_rex_inclusive_initialise_solving(slice_index si,
                                            circe_variant_type const *variant,
                                            slice_type interval_start);

/* Inialise check validation in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 */
void circe_rex_inclusive_initialise_check_validation(slice_index si);

#endif
