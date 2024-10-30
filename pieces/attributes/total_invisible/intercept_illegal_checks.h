#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_INTERCEPT_ILLEGAL_CHECKS_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_INTERCEPT_ILLEGAL_CHECKS_H

#include "position/side.h"
#include "position/board.h"

typedef void (deal_with_check_next_phase)(void);

void deal_with_illegal_checks(Side side_in_check, square king_pos, deal_with_check_next_phase *next);

#endif
