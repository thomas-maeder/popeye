#if !defined(PIECES_HOPPERS_H)
#define PIECES_HOPPERS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/vectors.h"

/* This module implements hopper pieces.
 */

void hoppers_generate_moves(square sq_departure,
                            vec_index_type kbeg, vec_index_type kend);

#endif
