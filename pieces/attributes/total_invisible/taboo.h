#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_TABOO_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_TABOO_H

#include "position/position.h"
#include "solving/ply.h"

boolean will_be_taboo(square s, Side side);
boolean was_taboo(square s, Side side);
boolean is_taboo(square s, Side side);

void update_nr_taboos_for_current_move_in_ply(int delta);

void update_nr_taboos_on_square(square s, int delta, ply ply);

boolean is_taboo_violation_acceptable(square first_taboo_violation);

square find_taboo_violation(void);

#endif
