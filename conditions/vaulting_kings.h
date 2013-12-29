#if !defined(CONDITIONS_VAULTING_KINGS_H)
#define CONDITIONS_VAULTING_KINGS_H

/* This module implements the fairy condition Vaulting Kings */

#include "pieces/pieces.h"
#include "position/position.h"
#include "solving/observation.h"
#include "utilities/boolean.h"

extern boolean vaulting_kings_transmuting[nr_sides];
extern PieNam king_vaulters[nr_sides][PieceCount];
extern unsigned int nr_king_vaulters[nr_sides];

/* Reset the king vaulters
 */
void reset_king_vaulters(void);

/* Append a piece to the king vaulters
 * @param side for who to add the piece?
 * @param p which piece to add?
 */
void append_king_vaulter(Side side, PieNam p);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void vaulting_kings_generate_moves_for_piece(slice_index si, PieNam p);

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean vaulting_king_is_square_observed(slice_index si, validator_id evaluate);

/* Make sure to behave correctly while detecting observations by vaulting kings
 */
boolean vaulting_kings_enforce_observer_walk(slice_index si);

/* Initialise the solving machinery with Vaulting Kings
 * @param si root slice of the solving machinery
 * @param side for whom
 */
void vaulting_kings_initalise_solving(slice_index si, Side side);

#endif
