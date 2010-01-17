#if !defined(STIPULATION_BRANCH_H)
#define STIPULATION_BRANCH_H

/* Functionality related to "pipe slices"; i.e. slices that have a
 * next member and whose functions normally delegate to those of the
 * next slice.
 */

#include "pystip.h"
#include "pyslice.h"

/* Allocate a new branch slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @param towards_goal identifies slice that leads towards goal from
 *                     the branch
 * @return newly allocated slice
 */
slice_index alloc_branch(SliceType type,
                         stip_length_type length,
                         stip_length_type min_length,
                         slice_index towards_goal);

/* Establish a link between a branch slice and its successor
 * @param branch identifies branch slice
 * @param succ identifies branch to become the successor
 */
void branch_link(slice_index branch, slice_index succ);

#endif
