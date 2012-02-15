#if !defined(CONDITIONS_ANTICIRCE_TARGET_SQUARE_FILTER_H)
#define CONDITIONS_ANTICIRCE_TARGET_SQUARE_FILTER_H

#include "pyslice.h"

/* This module provides slice type STAnticirceTargetSquareFilter - tests
 * additional conditions on some goals imposed by Anticirce:
 * - the mated side must have >=1 move that only fails because of self-check
 */

/* Allocate a STAnticirceTargetSquareFilter slice.
 * @param target target square to be reached
 * @return index of allocated slice
 */
slice_index alloc_anticirce_target_square_filter_slice(square target);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_target_square_filter_solve(slice_index si);

#endif
