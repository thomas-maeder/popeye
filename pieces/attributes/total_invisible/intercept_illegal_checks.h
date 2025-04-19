#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_INTERCEPT_ILLEGAL_CHECKS_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_INTERCEPT_ILLEGAL_CHECKS_H

#include "position/side.h"
#include "position/board.h"

typedef enum
{
  done_protecting_king_forward,
  done_protecting_king_backward
} done_protecting_king_direction;

typedef void (done_protecting_king_type)(done_protecting_king_direction direction);

void forward_protect_king(Side side_in_check,
                          square king_pos,
                          done_protecting_king_type *done_protecting_king);

#endif
