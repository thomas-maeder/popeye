#if !defined(CONDITIONS_TRANSMUTING_KINGS_REFLECTIVE_KINGS_H)
#define CONDITIONS_TRANSMUTING_KINGS_REFLECTIVE_KINGS_H

/* This module implements the Reflective Kings */

#include "pieces/pieces.h"
#include "position/side.h"
#include "stipulation/stipulation.h"

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void reflective_kings_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with reflective kings
 * @param si identifies root slice of solving machinery
 * @param side for whom
 */
void reflective_kings_initialise_solving(slice_index si, Side side);

/* Validate an observation according to Reflective Kings
 * @param si identifies next slice
 * @return true iff observation is valid
 */
boolean reflective_kings_enforce_observer_walk(slice_index si);

#endif
