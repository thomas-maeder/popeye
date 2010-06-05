#if !defined(OPTIMISATIONS_MAXSOLUTIONS_ROOT_SOLVABLE_FILTER_H)
#define OPTIMISATIONS_MAXSOLUTIONS_ROOT_SOLVABLE_FILTER_H

#include "py.h"
#include "pyslice.h"

/* This module provides functionality dealing with
 * STMaxSolutionsRootSolvableFilter stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsRootSolvableFilter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_root_solvable_filter(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maxsolutions_root_solvable_filter_solve(slice_index si);

#endif
