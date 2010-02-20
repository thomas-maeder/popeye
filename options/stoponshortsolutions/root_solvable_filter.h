#if !defined(OPTIMISATIONS_STOPONSHORTSOLUTIONS_ROOT_SOLVABLE_FILTER_H)
#define OPTIMISATIONS_STOPONSHORTSOLUTIONS_ROOT_SOLVABLE_FILTER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with
 * STStopOnShortSolutionsRootSolvableFilter stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STStopOnShortSolutionsRootSolvableFilter slice.
 * @return allocated slice
 */
slice_index alloc_stoponshortsolutions_root_solvable_filter(void);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean stoponshortsolutions_root_solvable_filter_root_solve(slice_index si);

#endif
