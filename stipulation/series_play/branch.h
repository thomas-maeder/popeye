#if !defined(STIPULATION_SERIES_PLAY_BRANCH_H)
#define STIPULATION_SERIES_PLAY_BRANCH_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STSeries*
 * stipulation slices.
 */

/* Shorten a series pipe by a half-move
 * @param pipe identifies pipe to be shortened
 */
void shorten_series_pipe(slice_index pipe);

/* Allocate a series branch where the next slice's starter is the
 * opponent of the series's starter. 
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of adapter slice of allocated series branch
 */
slice_index alloc_series_branch(stip_length_type length,
                                stip_length_type min_length,
                                slice_index proxy_to_goal);

#endif
