#if !defined(STIPULATION_SERIES_PLAY_END_OF_BRANCH_H)
#define STIPULATION_SERIES_PLAY_END_OF_BRANCH_H

#include "stipulation/series_play/play.h"

/* This module provides functionality dealing with the defending side
 * in STEndOfSeriesBranch stipulation slices.
 */

/* Allocate a STEndOfSeriesBranch slice.
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_end_of_series_branch_slice(slice_index proxy_to_next);

/* Traversal of the moves beyond a series fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_end_of_series_branch(slice_index si,
                                              stip_moves_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type end_of_series_branch_series(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type end_of_series_branch_has_series(slice_index si,
                                                 stip_length_type n);

#endif
