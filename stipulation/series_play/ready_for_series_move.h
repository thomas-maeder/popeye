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

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void ready_for_series_move_make_root(slice_index si,
                                     stip_structure_traversal *st);

/* Is it a dummy move that we are ready for?
 * @param si identifies slice
 */
boolean ready_for_series_move_is_move_dummy(slice_index si);

#endif
