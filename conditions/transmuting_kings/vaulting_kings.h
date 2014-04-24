#if !defined(CONDITIONS_VAULTING_KINGS_H)
#define CONDITIONS_VAULTING_KINGS_H

/* This module implements the fairy condition Vaulting Kings */

#include "pieces/pieces.h"
#include "position/side.h"
#include "solving/observation.h"
#include "utilities/boolean.h"

extern boolean vaulting_kings_transmuting[nr_sides];
extern piece_walk_type king_vaulters[nr_sides][nr_piece_walks];
extern unsigned int nr_king_vaulters[nr_sides];

/* Reset the king vaulters
 */
void reset_king_vaulters(void);

/* Append a piece to the king vaulters
 * @param side for who to add the piece?
 * @param p which piece to add?
 */
void append_king_vaulter(Side side, piece_walk_type p);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void vaulting_kings_generate_moves_for_piece(slice_index si);

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @note sets observation_validation_result
 */
void vaulting_king_is_square_observed(slice_index si);

/* Make sure to behave correctly while detecting observations by vaulting kings
 */
boolean vaulting_kings_enforce_observer_walk(slice_index si);

/* Initialise the solving machinery with Vaulting Kings
 * @param si root slice of the solving machinery
 * @param side for whom
 */
void vaulting_kings_initalise_solving(slice_index si, Side side);

#endif
