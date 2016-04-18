#if !defined(PIECES_WALKS_ARGENTINIAN_RIDERS_H)
#define PIECES_WALKS_ARGENTINIAN_RIDERS_H

/* This module implements argentinian rider pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"

/* Generate moves for an argentinian rider piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void argentinian_rider_generate_moves(vec_index_type kbeg, vec_index_type kend);

boolean senora_check(validator_id evaluate);
boolean faro_check(validator_id evaluate);
boolean loco_check(validator_id evaluate);

#endif
