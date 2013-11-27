#if !defined(PIECES_WALKS_RIDERS_H)
#define PIECES_WALKS_RIDERS_H

/* This module implements rider pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"

/* Generate moves to the square on a line segment
 * @param sq_base first square of line segment
 * @param k vector index indicating the direction of the line segment
 */
square generate_moves_on_line_segment(square sq_base, vec_index_type k);

/* Generate moves for a rider piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void rider_generate_moves(vec_index_type kbeg, vec_index_type kend);

boolean riders_check(vec_index_type b, vec_index_type c, validator_id evaluate);

boolean rook_check(validator_id evaluate);
boolean queen_check(validator_id evaluate);
boolean bishop_check(validator_id evaluate);
boolean nightrider_check(validator_id evaluate);
boolean elephant_check(validator_id evaluate);
boolean waran_check(validator_id evaluate);
boolean camel_rider_check(validator_id evaluate);
boolean zebra_rider_check(validator_id evaluate);
boolean gnu_rider_check(validator_id evaluate);

#endif
