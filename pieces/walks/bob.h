#if !defined(PIECES_WALKS_BOB_H)
#define PIECES_WALKS_BOB_H

/* This module implements the Bob */

#include "position/board.h"

/* Generate moves for an Bob
 * @param sq_departure common departure square of the generated moves
 */
void bob_generate_moves(square sq_departure);

#endif
