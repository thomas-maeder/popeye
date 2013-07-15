#if !defined(PIECES_WALKS_BOUNCER_H)
#define PIECES_WALKS_BOUNCER_H

/* This module implements the Bouncer */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate moves for an Bouncer
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void bouncer_generate_moves(square sq_departure,
                            vec_index_type kbeg, vec_index_type kend);

#endif
