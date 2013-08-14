#if !defined(PIECES_WALKS_SPIRAL_SPRINGERS_H)
#define PIECES_WALKS_SPIRAL_SPRINGERS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"

/* Generation of moves for Spiral springers. */

void spiralspringer_generate_moves(void);
boolean spiralspringer_check(evalfunction_t *evaluate);

void diagonalspiralspringer_generate_moves(void);
boolean diagonalspiralspringer_check(evalfunction_t *evaluate);

void boyscout_generate_moves(void);
boolean boyscout_check(evalfunction_t *evaluate);

void girlscout_generate_moves(void);
boolean girlscout_check(evalfunction_t *evaluate);

void spiralspringer40_generate_moves(void);
boolean spiralspringer40_check(evalfunction_t *evaluate);

void spiralspringer20_generate_moves(void);
boolean spiralspringer20_check(evalfunction_t *evaluate);

void spiralspringer33_generate_moves(void);
boolean spiralspringer33_check(evalfunction_t *evaluate);

void spiralspringer11_generate_moves(void);
boolean spiralspringer11_check(evalfunction_t *evaluate);

void quintessence_generate_moves(void);
boolean quintessence_check(evalfunction_t *evaluate);

#endif
