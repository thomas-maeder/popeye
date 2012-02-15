#if !defined(STIPULATION_CIRCE_STEINGEWINN_FILTER_H)
#define STIPULATION_CIRCE_STEINGEWINN_FILTER_H

#include "pyslice.h"

/* This module provides slice type STCirceSteingewinnFilter - detects whether a
 *  capture isn't a Steingewinn because the captured piece is reborn
 */

/* Allocate a STCirceSteingewinnFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_circe_steingewinn_filter_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type circe_steingewinn_filter_solve(slice_index si);

#endif
