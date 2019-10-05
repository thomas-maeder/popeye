#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_TABOO_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_TABOO_H

#include "position/position.h"
#include "solving/ply.h"

extern unsigned int nr_taboos_accumulated_until_ply[nr_sides][maxsquare];

extern unsigned int nr_taboos_for_current_move_in_ply[maxply+1][nr_sides][maxsquare];

boolean has_been_taboo_since_random_move(square s);

boolean is_taboo(square s, Side side);
boolean was_taboo(square s);

void update_nr_taboos_for_current_move_in_ply(int delta);

void update_taboo(int delta);

#endif
