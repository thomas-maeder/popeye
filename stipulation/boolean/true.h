#if !defined(STIPULATION_LEAF_H)
#define STIPULATION_LEAF_H

#include "pyslice.h"

/* This module provides functionality dealing with leaf slices
 */

/* Allocate a STGoalReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_leaf_slice(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter 
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_solve(slice_index si);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void leaf_insert_root(slice_index si, stip_structure_traversal *st);

#endif
