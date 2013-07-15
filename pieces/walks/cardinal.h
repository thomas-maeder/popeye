#if !defined(PIECES_WALKS_CARDINAL_H)
#define PIECES_WALKS_CARDINAL_H

/* This module implements the Cardinal */

#include "position/board.h"

/* Generate moves for a Cardinal
 * @param sq_departure common departure square of the generated moves
 */
void cardinal_generate_moves(square sq_departure);

#endif
