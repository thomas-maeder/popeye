#if !defined(PIECES_WALKS_LIONS_H)
#define PIECES_WALKS_LIONS_H

/* This module implements lion pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate moves for a lion piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void lions_generate_moves(vec_index_type kbeg, vec_index_type kend);

#endif
