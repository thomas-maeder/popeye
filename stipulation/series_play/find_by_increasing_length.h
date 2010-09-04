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
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_root_slice(stip_length_type length,
                                    stip_length_type min_length);

/* Traversal of the moves beyond a series root slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_root(slice_index si, stip_moves_traversal *st);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_root_solve(slice_index si);

#endif
