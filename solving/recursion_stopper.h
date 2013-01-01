#if !defined(SOLVING_RECURSION_STOPPER_H)
#define SOLVING_RECURSION_STOPPER_H

/* This module implements the slice type STRecursionStopper which can be used
 * to avoid infinite recursion in chess forms that allow for it
 */

#include "solving/solve.h"

/* Allocate a STRecursionStopper slice
 * @return newly allocated slice
 */
slice_index alloc_recursion_stopper_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type recursion_stopper_solve(slice_index si, stip_length_type n);

#endif
