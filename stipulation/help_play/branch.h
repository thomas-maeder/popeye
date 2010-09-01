#if !defined(STIPULATION_HELP_PLAY_BRANCH_H)
#define STIPULATION_HELP_PLAY_BRANCH_H

#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with STHelp*
 * stipulation slices.
 */

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_help_branch_to_goal(stip_length_type length,
                                      stip_length_type min_length,
                                      slice_index proxy_to_goal);
slice_index alloc_help_branch_not_to_goal(stip_length_type length,
                                          stip_length_type min_length,
                                          slice_index proxy_to_goal);

/* Shorten a help slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void help_branch_shorten_slice(slice_index si);

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 * @return entry slice of shortened branch
 *         no_slice if shortening isn't applicable
 */
slice_index help_branch_shorten(slice_index si);

#endif
