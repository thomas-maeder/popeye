#if !defined(OPTIMISATIONS_INTELLIGENT_GUARDS_FLIGHTS_H)
#define OPTIMISATIONS_INTELLIGENT_GUARDS_FLIGHTS_H

#include "py.h"

void intelligent_continue_guarding_flights(unsigned int nr_remaining_white_moves,
                                           unsigned int nr_remaining_black_moves,
                                           stip_length_type n,
                                           unsigned int index_of_next_guarding_piece,
                                           unsigned int min_nr_captures_by_white);

void intelligent_guard_flights(unsigned int nr_remaining_white_moves,
                               unsigned int nr_remaining_black_moves,
                               stip_length_type n,
                               unsigned int min_nr_captures_by_white);

#endif
