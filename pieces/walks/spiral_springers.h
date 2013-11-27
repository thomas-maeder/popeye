#if !defined(PIECES_WALKS_SPIRAL_SPRINGERS_H)
#define PIECES_WALKS_SPIRAL_SPRINGERS_H

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"

/* Generation of moves for Spiral springers. */

void spiralspringer_generate_moves(void);
boolean spiralspringer_check(validator_id evaluate);

void diagonalspiralspringer_generate_moves(void);
boolean diagonalspiralspringer_check(validator_id evaluate);

void boyscout_generate_moves(void);
boolean boyscout_check(validator_id evaluate);

void girlscout_generate_moves(void);
boolean girlscout_check(validator_id evaluate);

void spiralspringer40_generate_moves(void);
boolean spiralspringer40_check(validator_id evaluate);

void spiralspringer20_generate_moves(void);
boolean spiralspringer20_check(validator_id evaluate);

void spiralspringer33_generate_moves(void);
boolean spiralspringer33_check(validator_id evaluate);

void spiralspringer11_generate_moves(void);
boolean spiralspringer11_check(validator_id evaluate);

void quintessence_generate_moves(void);
boolean quintessence_check(validator_id evaluate);

#endif
