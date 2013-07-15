#if !defined(PIECES_WALKS_LEAPERS_H)
#define PIECES_WALKS_LEAPERS_H

/* This module implements leaper pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate moves for a leaper piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void leaper_generate_moves(square sq_departure, vec_index_type kbeg, vec_index_type kend);

#endif
