#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_RANDOM_MOVE_BY_INVISIBLE_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_RANDOM_MOVE_BY_INVISIBLE_H

#include "position/position.h"
#include "solving/ply.h"

void flesh_out_random_move_by_invisible(void);

boolean is_random_move_by_invisible(ply ply);

void backward_fleshout_random_move_by_invisible(void);

square const *find_next_forward_mover(square const *start_square);

#endif
