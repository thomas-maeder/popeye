#if !defined(PYCONST_H)
#define PYCONST_H

#include "boolean.h"
#include "pyslice.h"

/* This module provides functionality dealing with leaf slices with
 * constant boolean value (i.e. that either solve always or never).
 */

/* Allocate a slice with constant value.
 * @param value constant value
 * @return index of allocated slice
 */
slice_index alloc_constant_slice(boolean value);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean constant_has_solution(slice_index si);

#endif
