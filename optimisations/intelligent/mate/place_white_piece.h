#if !defined(OPTIMISATION_INTELLIGENT_MATE_PLACE_WHITE_PIECE_H)
#define OPTIMISATION_INTELLIGENT_MATE_PLACE_WHITE_PIECE_H

#include "py.h"
#include "pyboard.h"

void intelligent_mate_place_any_white_piece_on(unsigned int nr_remaining_white_moves,
                                               unsigned int nr_remaining_black_moves,
                                               unsigned int max_nr_allowed_captures_by_white,
                                               unsigned int max_nr_allowed_captures_by_black,
                                               stip_length_type n,
                                               square placed_on);

#endif
