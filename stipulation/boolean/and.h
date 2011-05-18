#if !defined(PYRECIPR_H)
#define PYRECIPR_H

#include "pyslice.h"

/* This module provides functionality dealing logical AND stipulation slices.
 */

/* Allocate a STAnd slice.
 * @param proxy1 proxy to 1st operand
 * @param proxy2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_and_slice(slice_index proxy1, slice_index proxy2);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type and_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type and_solve(slice_index si);

#endif
