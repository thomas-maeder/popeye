#if !defined(CONDITIONS_MARSCIRCE_PLUS_H)
#define CONDITIONS_MARSCIRCE_PLUS_H

#include "pieces/pieces.h"
#include "solving/machinery/solve.h"
#include "solving/observation.h"

/* This module provides implements the condition Echecs Plus
 */

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void plus_generate_additional_captures_for_piece(slice_index si);

/* Determine whether a side observes a specific square
 * @param side_observing the side
 * @return true iff side is in check
 */
boolean plus_is_square_observed(slice_index si, validator_id evaluate);

/* Inialise thet solving machinery with Plus Chess
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_plus(slice_index si);

#endif
