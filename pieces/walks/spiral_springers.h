#if !defined(PIECES_WALKS_SPIRAL_SPRINGERS_H)
#define PIECES_WALKS_SPIRAL_SPRINGERS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generation of moves for Spiral springers. */

void spiralspringer_generate_moves(void);

void diagonalspiralspringer_generate_moves(void);

void boyscout_generate_moves(void);

void girlscout_generate_moves(void);

void spiralspringer40_generate_moves(void);
void spiralspringer20_generate_moves(void);
void spiralspringer33_generate_moves(void);
void spiralspringer11_generate_moves(void);
void quintessence_generate_moves(void);

#endif
