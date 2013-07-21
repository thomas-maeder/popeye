#if !defined(SOLVING_FIND_SQUARE_OBSERVER_TRACKING_BACK_FROM_TARGET_H)
#define SOLVING_FIND_SQUARE_OBSERVER_TRACKING_BACK_FROM_TARGET_H

#include "utilities/boolean.h"
#include "pyproc.h"

boolean reflective_king_is_square_observed(slice_index si,
                                                            square sq_target,
                                                            evalfunction_t *evaluate);

boolean find_square_observer_tracking_back_from_target_king(slice_index si,
                                                            square sq_target,
                                                            evalfunction_t *evaluate);

boolean find_square_observer_tracking_back_from_target(slice_index si,
                                                       square sq_target,
                                                       evalfunction_t *evaluate);

#endif
