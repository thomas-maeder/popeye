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
void phantom_enforce_rex_inclusive(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_avoid_duplicate_moves(slice_index si);

/* Inialise the solving machinery with Phantom Chess
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_phantom(slice_index si);

/* Determine whether a specific side is in check in Phantom Chess
 * @param si identifies tester slice
 * @note sets observation_validation_result
 */
void phantom_is_square_observed(slice_index si);

#endif
