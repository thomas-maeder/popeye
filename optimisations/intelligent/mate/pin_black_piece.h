#if !defined(OPTIMISATION_INTELLIGENT_MATE_PIN_BLACK_PIECE_H)
#define OPTIMISATION_INTELLIGENT_MATE_PIN_BLACK_PIECE_H

#include "py.h"
#include "pyboard.h"

void intelligent_mate_pin_black_piece(unsigned int nr_remaining_black_moves,
                                      unsigned int nr_remaining_white_moves,
                                      unsigned int max_nr_allowed_captures_by_black_pieces,
                                      unsigned int max_nr_allowed_captures_by_white_pieces,
                                      stip_length_type n,
                                      square sq_to_be_pinned);

#endif
