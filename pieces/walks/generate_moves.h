#if !defined(PIECES_GENERATE_MOVES_H)
#define PIECES_GENERATE_MOVES_H

/* This module generates moves for pieces based on their walk */

#include "position/position.h"

/* Generate moves for a piece based on its walk
 * @param p the piece's walk
 */
void generate_moves_for_piece_based_on_walk(PieNam p);

#endif
