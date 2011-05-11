#if !defined(STIPULATION_LEAF_H)
#define STIPULATION_LEAF_H

#include "pyslice.h"

/* This module provides functionality dealing with true slices
 */

/* Allocate a STTrue slice.
 * @return index of allocated slice
 */
slice_index alloc_true_slice(void);

/* Determine whether a slice has a solution
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type true_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type true_solve(slice_index si);

#endif
