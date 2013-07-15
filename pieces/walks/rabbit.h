#if !defined(PIECES_WALKS_RABBIT_H)
#define PIECES_WALKS_RABBIT_H

/* This module implements the Rabbit */

#include "position/board.h"

/* Generate moves for an Rabbit
 * @param sq_departure common departure square of the generated moves
 */
void rabbit_generate_moves(square sq_departure);

#endif
