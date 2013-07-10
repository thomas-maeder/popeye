#if !defined(PIECES_ROSESS_H)
#define PIECES_ROSESS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/vectors.h"

/* Generation of moves for Spiral springers. */

void spiralspringer_generate_moves(square sq_departure);

void diagonalspiralspringer_generate_moves(square sq_departure);

void boyscout_generate_moves(square sq_departure);

void girlscout_generate_moves(square sq_departure);

#endif
