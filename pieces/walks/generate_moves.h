#if !defined(PIECES_GENERATE_MOVES_H)
#define PIECES_GENERATE_MOVES_H

/* This module generates moves for pieces based on their walk */

#include "position/board.h"
#include "position/position.h"

/* Generate moves for a piece based on its walk
 * @param sq_departure common departure square of the generated moves
 * @param p the piece's walk
 */
void generate_moves_for_piece_based_on_walk(square sq_departure, PieNam p);

#endif
