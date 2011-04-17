#if !defined(STIPULATION_HELP_PLAY_FIND_SHORTEST_H)
#define STIPULATION_HELP_PLAY_FIND_SHORTEST_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STHelpFindShortest
 * stipulation slices.
 */

/* Allocate a STHelpFindShortest slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_find_shortest_slice(stip_length_type length,
                                           stip_length_type min_length);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */

void help_find_shortest_make_root(slice_index si,
                                  stip_structure_traversal *st);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type help_find_shortest_help(slice_index si, stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type help_find_shortest_can_help(slice_index si, stip_length_type n);

#endif
