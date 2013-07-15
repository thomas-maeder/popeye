#if !defined(PIECES_WALKS_SPIRAL_SPRINGERS_H)
#define PIECES_WALKS_SPIRAL_SPRINGERS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generation of moves for Spiral springers. */

void spiralspringer_generate_moves(square sq_departure);

void diagonalspiralspringer_generate_moves(square sq_departure);

void boyscout_generate_moves(square sq_departure);

void girlscout_generate_moves(square sq_departure);

void spiralspringer40_generate_moves(square sq_departure);
void spiralspringer20_generate_moves(square sq_departure);
void spiralspringer33_generate_moves(square sq_departure);
void spiralspringer11_generate_moves(square sq_departure);
void quintessence_generate_moves(square sq_departure);

#endif
