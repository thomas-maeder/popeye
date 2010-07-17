#if !defined(STIPULATION_SERIES_PLAY_OR_H)
#define STIPULATION_SERIES_PLAY_OR_H

/* This module provides functionality dealing with STSeriesOR
 * stipulation slices.
 */

#include "pyslice.h"

/* Allocate a STSeriesOR slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_series_OR_slice(stip_length_type length,
                                  stip_length_type min_length,
                                  slice_index proxy_to_goal);

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_series_OR(slice_index branch,
                                       stip_structure_traversal *st);

/* Traversal of the moves beyond a series OR slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_OR(slice_index si, stip_move_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_OR_solve_in_n(slice_index si, stip_length_type n);

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
stip_length_type series_OR_has_solution_in_n(slice_index si,
                                             stip_length_type n);

#endif
