#if !defined(OPTIMISATION_INTELLIGENT_MATE_INTERCEPT_CHECKS_H)
#define OPTIMISATION_INTELLIGENT_MATE_INTERCEPT_CHECKS_H

#include "py.h"

void intelligent_mate_intercept_checks(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n);

#endif
