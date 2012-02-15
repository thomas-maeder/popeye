#if !defined(STIPULATION_BOOLEAN_FALSE_H)
#define STIPULATION_BOOLEAN_FALSE_H

#include "pyslice.h"

/* Allocate a STFalse slice.
 * @return index of allocated slice
 */
slice_index alloc_false_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type false_solve(slice_index si);

#endif
