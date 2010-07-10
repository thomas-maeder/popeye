#if !defined(OPTIMISATIONS_MAXSOLUTIONS_SOLVABLE_FILTER_H)
#define OPTIMISATIONS_MAXSOLUTIONS_SOLVABLE_FILTER_H

#include "pyslice.h"

/* This module provides functionality dealing with
 * STMaxSolutionsSolvableFilter stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsSolvableFilter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_solvable_filter(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter 
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maxsolutions_solvable_filter_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maxsolutions_solvable_filter_solve(slice_index si);

#endif
