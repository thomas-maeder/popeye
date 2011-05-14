#if !defined(STIPULATION_HELP_PLAY_FIND_BY_INCREASING_LENGTH_H)
#define STIPULATION_HELP_PLAY_FIND_BY_INCREASING_LENGTH_H

#include "stipulation/help_play/play.h"
#include "stipulation/series_play/play.h"

/* This module provides functionality dealing with STFindByIncreasingLength
 * stipulation slices.
 */

/* Allocate a STFindByIncreasingLength slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_find_by_increasing_length_slice(stip_length_type length,
                                                  stip_length_type min_length);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type find_by_increasing_length_help(slice_index si,
                                                stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type find_by_increasing_length_can_help(slice_index si,
                                                    stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type find_by_increasing_length_series(slice_index si,
                                                  stip_length_type n);

#endif
