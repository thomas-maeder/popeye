#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_RANDOM_MOVE_BY_INVISIBLE_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_RANDOM_MOVE_BY_INVISIBLE_H

#include "position/position.h"
#include "solving/ply.h"

void flesh_out_random_move_by_invisible(void);

void flesh_out_random_move_by_specific_invisible_to(square sq_arrival);

boolean is_random_move_by_invisible(ply ply);

void retract_random_move_by_invisible(square const *start_square);

void backward_fleshout_random_move_by_invisible(void);

#endif
