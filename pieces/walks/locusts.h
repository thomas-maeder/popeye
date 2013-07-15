#if !defined(PIECES_WALKS_LOCUST_H)
#define PIECES_WALKS_LOCUST_H

/* This module implements locust pieces */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate a single Locust capture
 * @param sq_departure departure square
 * @param sq_capture capture square
 * @param index_arrival_capturek identifies the vector from capture to arrival square
 */
void generate_locust_capture(square sq_departure, square sq_capture,
                             vec_index_type index_arrival_capture);

/* Generate moves for an Locust piece
 * @param sq_departure common departure square of the generated moves
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void locust_generate_moves(square sq_departure,
                           vec_index_type kbeg, vec_index_type kend);

#endif
