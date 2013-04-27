#if !defined(PIECES_HUNTERS_H)
#define PIECES_HUNTERS_H

#include "position/board.h"
#include "position/position.h"
#include "py.h"

void hunter_generate_moves(Side side, square sq_departure, PieNam walk);

void rook_hunter_generate_moves(Side side, square sq_departure);
void bishop_hunter_generate_moves(Side side, square sq_departure);

#endif
