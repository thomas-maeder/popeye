#if !defined(PIECESS_PAWNS_PAWN_H)
#define PIECESS_PAWNS_PAWN_H

#include "position/board.h"
#include "position/position.h"

unsigned int pawn_get_no_capture_length(Side side, square sq_departure);

void  pawn_generate_moves(Side side, square sq_departure);

#endif
