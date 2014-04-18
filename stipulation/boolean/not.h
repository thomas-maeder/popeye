#if !defined(STIPULATION_BOOLEAN_NOT_H)
#define STIPULATION_BOOLEAN_NOT_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing with STNot stipulation
 * slices.
 */

/* Allocate a not slice.
 * @return index of allocated slice
 */
slice_index alloc_not_slice(void);

#endif
