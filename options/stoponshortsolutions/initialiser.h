#if !defined(OPTIONS_STOPONSHORTSOLUTIONS_H)
#define OPTIONS_STOPONSHORTSOLUTIONS_H

#include "py.h"
#include "solving/solve.h"

/* This module provides functionality dealing with
 * STStopOnShortSolutionsInitialiser stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STStopOnShortSolutionsInitialiser slice.
 * @return allocated slice
 */
slice_index alloc_stoponshortsolutions_initialiser_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type stoponshortsolutions_initialiser_solve(slice_index si, stip_length_type n);

#endif
