#if !defined(PIECES_WALKS_CAT_H)
#define PIECES_WALKS_CAT_H

/* This module implements the Cat */

#include "position/board.h"

/* Generate moves for a rider piece
 * @param sq_departure common departure square of the generated moves
 */
void cat_generate_moves(square sq_departure);

#endif
