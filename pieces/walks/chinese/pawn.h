#if !defined(PIECES_WALKS_CHINESE_PAWN_H)
#define PIECES_WALKS_CHINESE_PAWN_H

/* This module implements chinese pawns */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for a chinese pawn piece
 */
void chinese_pawn_generate_moves(void);

boolean chinese_pawn_check(evalfunction_t *evaluate);

#endif
