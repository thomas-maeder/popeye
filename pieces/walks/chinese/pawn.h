#if !defined(PIECES_WALKS_CHINESE_PAWN_H)
#define PIECES_WALKS_CHINESE_PAWN_H

/* This module implements chinese pawns */

#include "position/board.h"

/* Generate moves for a chinese pawn piece
 * @param sq_departure common departure square of the generated moves
 */
void chinese_pawn_generate_moves(square sq_departure);

#endif
