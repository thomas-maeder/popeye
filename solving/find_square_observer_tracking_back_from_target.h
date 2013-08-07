#if !defined(SOLVING_FIND_SQUARE_OBSERVER_TRACKING_BACK_FROM_TARGET_H)
#define SOLVING_FIND_SQUARE_OBSERVER_TRACKING_BACK_FROM_TARGET_H

#include "utilities/boolean.h"
#include "pyproc.h"

extern PieNam observing_walk[maxply+1];

boolean find_square_observer_tracking_back_from_target_king(slice_index si,
                                                            evalfunction_t *evaluate);

boolean find_square_observer_tracking_back_from_target_non_king(slice_index si,
                                                                evalfunction_t *evaluate);

boolean find_square_observer_tracking_back_from_target_fairy(slice_index si,
                                                             evalfunction_t *evaluate);

#endif
