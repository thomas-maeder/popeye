#if !defined(PIECES_WALKS_CHINESE_RIDERS_H)
#define PIECES_WALKS_CHINESE_RIDERS_H

/* This module implements chinese rider pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate moves for a chinese rider piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void chinese_rider_generate_moves(square sq_departure,
                                  vec_index_type kbeg, vec_index_type kend);

#endif
