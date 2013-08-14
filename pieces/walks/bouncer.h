#if !defined(PIECES_WALKS_BOUNCER_H)
#define PIECES_WALKS_BOUNCER_H

/* This module implements the Bouncer */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"

/* Generate moves for an Bouncer
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void bouncer_generate_moves(vec_index_type kbeg, vec_index_type kend);

boolean bouncer_check(evalfunction_t *evaluate);

boolean rookbouncer_check(evalfunction_t *evaluate);

boolean bishopbouncer_check(evalfunction_t *evaluate);

#endif
