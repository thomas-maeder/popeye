#if !defined(CONDITIONS_CIRCE_REBIRTH_SQUARE_OCCUPIED_H)
#define CONDITIONS_CIRCE_REBIRTH_SQUARE_OCCUPIED_H

/* This module supports dealing with the situation when a Circe rebirth square
 * is occupied
 */

#include "stipulation/stipulation.h"

struct circe_variant_type;

typedef enum
{
  circe_on_occupied_rebirth_square_default,
  circe_on_occupied_rebirth_square_relaxed,
  circe_on_occupied_rebirth_square_strict,
  circe_on_occupied_rebirth_square_assassinate,
  circe_on_occupied_rebirth_square_volcanic,
  circe_on_occupied_rebirth_square_parachute
} circe_behaviour_on_occupied_rebirth_square_type;

/* Retrieve the behaviour of a Circe variant if the rebirth square is occupied
 * @param variant address of the structure holding the variant
 * @return the enumerator identifying the behaviour
 */
circe_behaviour_on_occupied_rebirth_square_type
circe_get_on_occupied_rebirth_square(struct circe_variant_type const *variant);

/* Deal with the situation where a rebirth is to occur on an occupied square
 * @param si identifies entry slice into solving machinery
 * @param variant identifies address of structure holding the Circe variant
 * @param interval_start type of slice that starts the sequence of slices
 *                       implementing that variant
 */
void circe_solving_instrument_rebirth_on_occupied_square(slice_index si,
                                                         struct circe_variant_type const *variant,
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
void circe_test_rebirth_square_empty_solve(slice_index si);

#endif
