#if !defined(OPTIMISATION_INTELLIGENT_MATE_FINISH_H)
#define OPTIMISATION_INTELLIGENT_MATE_FINISH_H

#include "py.h"

void intelligent_mate_test_target_position(unsigned int nr_remaining_white_moves,
                                           unsigned int nr_remaining_black_moves,
                                           unsigned int max_nr_allowed_captures_by_white,
                                           unsigned int max_nr_allowed_captures_by_black,
                                           stip_length_type n);

void intelligent_mate_finish(unsigned int nr_remaining_white_moves,
                             unsigned int nr_remaining_black_moves,
                             unsigned int max_nr_allowed_captures_by_white,
                             unsigned int max_nr_allowed_captures_by_black,
                             stip_length_type n,
                             unsigned int nr_of_checks_to_white);

#endif
