#if !defined(PYLEAFF_H)
#define PYLEAFF_H

#include "pyslice.h"

/* This module provides functionality dealing with leaf slices that
 * detect whether the defending side has just the goal that it defends
 * against.
 */

/* Determine whether a leaf slice.has just been solved with the move
 * by the non-starter 
 * @param leaf slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_forced_has_solution(slice_index leaf);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_forced_solve(slice_index leaf);

#endif
