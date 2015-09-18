#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_WHITE_BLOCK_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_WHITE_BLOCK_H

#include "position/position.h"
#include "stipulation/stipulation.h"

/* Block a black pawn with a white piece
 * @param to_be_blocked where to block the pawn
 */
void intelligent_stalemate_white_block(slice_index si, square to_be_blocked);

#endif
