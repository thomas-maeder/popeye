#if !defined(STIPULATION_SERIES_PLAY_ADAPTER_H)
#define STIPULATION_SERIES_PLAY_ADAPTER_H

#include "pyslice.h"

/* This module provides functionality dealing with STSeriesAdapter
 * stipulation slices. STSeriesAdapter slices switch from general play to series
 * play.
 */

/* Allocate a STSeriesAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_adapter_slice(stip_length_type length,
                                       stip_length_type min_length);

#endif
