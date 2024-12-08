#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_INTERCEPT_ILLEGAL_CHECKS_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_INTERCEPT_ILLEGAL_CHECKS_H

#include "position/side.h"
#include "position/board.h"

typedef void (forward_from_protect_king_type)(void);

void forward_protect_king(Side side_in_check,
                          square king_pos,
                          forward_from_protect_king_type *forward_from_protect_king);

#endif
