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

boolean riders_check(vec_index_type b, vec_index_type c, evalfunction_t *evaluate);

boolean rook_check(evalfunction_t *evaluate);
boolean queen_check(evalfunction_t *evaluate);
boolean bishop_check(evalfunction_t *evaluate);
boolean nightrider_check(evalfunction_t *evaluate);
boolean elephant_check(evalfunction_t *evaluate);
boolean waran_check(evalfunction_t *evaluate);
boolean camel_rider_check(evalfunction_t *evaluate);
boolean zebra_rider_check(evalfunction_t *evaluate);
boolean gnu_rider_check(evalfunction_t *evaluate);

#endif
