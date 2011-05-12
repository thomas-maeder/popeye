#if !defined(STIPULATION_SERIES_PLAY_READY_FOR_SERIES_MOVE_H)
#define STIPULATION_SERIES_PLAY_READY_FOR_SERIES_MOVE_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STReadyForSeriesMove
 * stipulation slices.
 */

/* Allocate a STReadyForSeriesMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_series_move_slice(stip_length_type length,
                                              stip_length_type min_length);

#endif
