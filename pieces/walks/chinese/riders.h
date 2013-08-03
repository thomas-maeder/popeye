#if !defined(PIECES_WALKS_CHINESE_RIDERS_H)
#define PIECES_WALKS_CHINESE_RIDERS_H

/* This module implements chinese rider pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate moves for a chinese rider piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void chinese_rider_generate_moves(vec_index_type kbeg, vec_index_type kend);

#endif
