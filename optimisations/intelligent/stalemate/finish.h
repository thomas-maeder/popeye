#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_FINISH_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_FINISH_H

#include "py.h"
#include "pyboard.h"

void intelligent_stalemate_continue_after_block(stip_length_type n,
                                                Side side,
                                                square to_be_blocked,
                                                piece blocker_type,
                                                unsigned int nr_checks_to_opponent);

void intelligent_stalemate_test_target_position(stip_length_type n);

#endif
