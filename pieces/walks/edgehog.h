#if !defined(PIECES_WALKS_EDGEHOG_H)
#define PIECES_WALKS_EDGEHOG_H

/* This module implements the Edgehog */

#include "position/board.h"

/* Generate moves for an Edgehog
 * @param sq_departure common departure square of the generated moves
 */
void edgehog_generate_moves(square sq_departure);

#endif
