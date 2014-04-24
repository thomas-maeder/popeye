#if !defined(CONDITIONS_MARSCIRCE_MARSCIRCE_H)
#define CONDITIONS_MARSCIRCE_MARSCIRCE_H

#include "pieces/pieces.h"
#include "solving/machinery/solve.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "stipulation/structure_traversal.h"
#include "position/position.h"
#include "conditions/circe/circe.h"

/* This module provides implements the condition Mars-Circe
 */

extern square marscirce_rebirth_square[toppile+1];

extern square (*marscirce_determine_rebirth_square)(piece_walk_type, Flags, square, square, square, Side);

extern circe_variant_type marscirce_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void marscirce_reset_variant(circe_variant_type *variant);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_fix_departure(slice_index si);

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
void marscirce_initialise_reborn_from_generated_solve(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_remember_rebirth(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_remember_no_rebirth(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_remove_capturer_solve(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_generate_from_rebirth_square(slice_index si);

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
void marscirce_move_to_rebirth_square_solve(slice_index si);

/* Validate an observation or observer by making sure it's the one that has just
 * been reborn
 * @param si identifies the validator slice
 */
boolean mars_enforce_observer(slice_index si);

/* Determine whether a specific piece delivers check to a specific side from a
 * specific rebirth square
 * @param observer_origin potentially delivering check ...
 * @param sq_rebrirth ... from this square
 * @note the piece on observer_origin must belong to advers(side)
 */
boolean mars_is_square_observed_from_rebirth_square(slice_index si,
                                                    validator_id evaluate,
                                                    square observer_origin,
                                                    square sq_rebirth);

/* Determine whether a specific piece delivers check to a specific side
 * @param observer_origin potentially delivering check ...
 * @note the piece on pos_checking must belong to advers(side)
 */
boolean mars_is_square_observed_by(slice_index si,
                                    validator_id evaluate,
                                    square observer_origin);

/* Determine whether a side observes a specific square
 * @param side_observing the side
 * @note sets observation_validation_result
 */
void marscirce_is_square_observed(slice_index si);

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_marscirce(slice_index si);

#endif
