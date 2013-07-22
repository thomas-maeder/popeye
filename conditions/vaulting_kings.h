#if !defined(CONDITIONS_VAULTING_KINGS_H)
#define CONDITIONS_VAULTING_KINGS_H

/* This module implements the fairy condition Vaulting Kings */

#include "utilities/boolean.h"
#include "position/position.h"
#include "pyproc.h"

extern boolean vaulting_kings_transmuting[nr_sides];
extern PieNam king_vaulters[nr_sides][PieceCount];

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
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void vaulting_kings_generate_moves_for_piece(slice_index si,
                                             square sq_departure,
                                             PieNam p);

/* Initialise the solving machinery with Vaulting Kings
 * @param si root slice of the solving machinery
 */
void vaulting_kings_initalise_solving(slice_index si);

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @param sq_target the square
 * @return true iff sq_target is observed by the side at the move
 */
boolean vaulting_king_is_square_observed(slice_index si,
                                         square sq_target,
                                         evalfunction_t *evaluate);

#endif
