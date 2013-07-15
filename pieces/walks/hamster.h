#if !defined(PIECES_WALKS_HAMSTER_H)
#define PIECES_WALKS_HAMSTER_H

/* This module implements the Hamster */

#include "position/board.h"

/* Generate moves for an Hamster
 * @param sq_departure common departure square of the generated moves
 */
void hamster_generate_moves(square sq_departure);

#endif
