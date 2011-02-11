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

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_root_solve_in_n(slice_index si, stip_length_type n);

#endif
