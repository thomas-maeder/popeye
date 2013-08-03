#if !defined(CONDITIONS_TRANSMUTING_KINGS_REFLECTIVE_KINGS_H)
#define CONDITIONS_TRANSMUTING_KINGS_REFLECTIVE_KINGS_H

/* This module implements the Reflective Kings */

#include "position/position.h"
#include "stipulation/stipulation.h"
#include "pyproc.h"

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void reflective_kings_generate_moves_for_piece(slice_index si, PieNam p);

/* Inialise the solving machinery with reflective kings
 * @param si identifies root slice of solving machinery
 * @param side for whom
 */
void reflective_kings_initialise_solving(slice_index si, Side side);

/* Determine whether a square is observed be the side at the move according to
 * Reflective Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean reflective_king_is_square_observed(slice_index si, evalfunction_t *evaluate);

#endif
