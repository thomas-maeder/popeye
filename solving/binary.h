#if !defined(SOLVING_BINARY_H)
#define SOLVING_BINARY_H

/* Binary slices
 */

#include "solving/machinery/solve.h"

/* Solve one of the two legs of a binary slice
 * @param si identifies the binary slice
 * @param condition if true, solve next2, otherwise next1
 */
void binary_solve_if_then_else(slice_index si, boolean condition);

#endif
