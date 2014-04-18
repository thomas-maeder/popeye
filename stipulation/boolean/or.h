#if !defined(STIPULATION_BOOLEAN_OR_H)
#define STIPULATION_BOOLEAN_OR_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing logical OR stipulation slices.
 */

/* Allocate a STOr slice.
 * @param op1 proxy to 1st operand
 * @param op2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_or_slice(slice_index op1, slice_index op2);

#endif
