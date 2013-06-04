#if !defined(CONDITIONS_VAULTING_KINGS_H)
#define CONDITIONS_VAULTING_KINGS_H

/* This module implements the fairy condition Vaulting Kings */

#include "utilities/boolean.h"
#include "position/position.h"
#include "pyproc.h"

extern boolean calc_transmuting_king[nr_sides];
extern PieNam king_vaulters[nr_sides][PieceCount];

/* Reset the king vaulters
 */
void reset_king_vaulters(void);

/* Append a piece to the king vaulters
 * @param side for who to add the piece?
 * @param p which piece to add?
 */
void append_king_vaulter(Side side, PieNam p);

/* Does the king of side trait[nbply] attack a particular square
 * (while vaulting or not)?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * true iff the king attacks sq_target
 */
boolean vaulting_kings_is_square_attacked_by_king(square sq_target,
                                                  evalfunction_t *evaluate);

/* Generate moves for a vaulting king
 */
boolean vaulting_kings_generate_moves(Side side, square sq_departure);

/* Initialise solving in Vaulting Kings
 */
void vaulting_kings_initalise_solving(void);

#endif
