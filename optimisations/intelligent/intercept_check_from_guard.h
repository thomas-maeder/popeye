#if !defined(OPTIMISATIONS_INTELLIGENT_INTERCEPT_FROM_GUARD_H)
#define OPTIMISATIONS_INTELLIGENT_INTERCEPT_FROM_GUARD_H

#include "py.h"
#include "pyboard.h"

square where_to_intercept_check_from_guard(piece guard_type, square guard_from);

void intercept_check_on_guarded_square(stip_length_type n,
                                       unsigned int index_of_next_guarding_piece,
                                       square to_be_intercepted,
                                       unsigned int min_nr_captures_by_white);

#endif
