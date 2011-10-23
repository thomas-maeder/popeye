#if !defined(OPTIONS_MAXSOLUTIONS_INITIALISER_H)
#define OPTIONS_MAXSOLUTIONS_INITIALISER_H

#include "py.h"
#include "pyslice.h"

/* This module provides functionality dealing with
 * STMaxSolutionsInitialiser stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsInitialiser slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_initialiser_slice(void);

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void maxsolutions_initialiser_apply_setplay(slice_index si,
                                            stip_structure_traversal *st);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maxsolutions_initialiser_solve(slice_index si);

#endif
