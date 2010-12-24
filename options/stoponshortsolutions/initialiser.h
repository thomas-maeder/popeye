#if !defined(OPTIMISATIONS_STOPONSHORTSOLUTIONS_ROOT_SOLVABLE_FILTER_H)
#define OPTIMISATIONS_STOPONSHORTSOLUTIONS_ROOT_SOLVABLE_FILTER_H

#include "py.h"
#include "pyslice.h"

/* This module provides functionality dealing with
 * STStopOnShortSolutionsInitialiser stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STStopOnShortSolutionsInitialiser slice.
 * @return allocated slice
 */
slice_index alloc_stoponshortsolutions_initialiser_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type stoponshortsolutions_initialiser_solve(slice_index si);

#endif
