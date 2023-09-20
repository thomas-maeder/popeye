#if !defined(CONDITIONS_MARSCIRCE_PHANTOM_H)
#define CONDITIONS_MARSCIRCE_PHANTOM_H

#include "pieces/pieces.h"
#include "solving/machinery/solve.h"
#include "solving/observation.h"
#include "conditions/circe/circe.h"

/* This module provides implements the condition Phantom Chess
 */

extern circe_variant_type phantom_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void phantom_reset_variant(circe_variant_type *variant);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_avoid_duplicate_moves(slice_index si);

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean phantom_king_square_observation_tester_ply_initialiser_is_in_check(slice_index si,
                                                                           Side side_in_check);

/* Inialise the solving machinery with Phantom Chess
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_phantom(slice_index si);

#endif
