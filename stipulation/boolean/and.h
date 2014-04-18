#if !defined(STIPULATION_BOOLEAN_AND_H)
#define STIPULATION_BOOLEAN_AND_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing logical AND stipulation slices.
 */

/* Allocate a STAnd slice.
 * @param op1 proxy to 1st operand
 * @param op2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_and_slice(slice_index op1, slice_index op2);

#endif
