#if !defined(STIPULATION_SERIES_PLAY_FORK_H)
#define STIPULATION_SERIES_PLAY_FORK_H

/* This module provides functionality dealing with STSeriesFork
 * stipulation slices.
 */

#include "pyslice.h"

/* Allocate a STSeriesFork slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_series_fork_slice(slice_index to_goal);

/* Traversal of the moves beyond a series fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_fork(slice_index si, stip_moves_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_fork_series(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_fork_has_series(slice_index si, stip_length_type n);

#endif
