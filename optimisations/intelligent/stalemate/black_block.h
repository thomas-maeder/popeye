#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_BLACK_BLOCK_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_BLACK_BLOCK_H

#include "py.h"

void intelligent_stalemate_black_block(unsigned int nr_remaining_white_moves,
                                       unsigned int nr_remaining_black_moves,
                                       unsigned int max_nr_allowed_captures_by_white,
                                       unsigned int max_nr_allowed_captures_by_black,
                                       stip_length_type n,
                                       square to_be_blocked);

#endif
