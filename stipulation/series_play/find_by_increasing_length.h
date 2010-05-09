#if !defined(STIPULATION_SERIES_PLAY_ROOT_H)
#define STIPULATION_SERIES_PLAY_ROOT_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with STSeriesRoot
 * stipulation slices.
 */

/* Allocate a STSeriesRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param next identifies following branch silice
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_root_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index next,
                                    slice_index short_sols);

/* Solve a branch slice at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean series_root_root_solve(slice_index si);

#endif
