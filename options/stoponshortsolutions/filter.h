#if !defined(OPTIONS_STOPONSHORTSOLUTIONS_FILTER_H)
#define OPTIONS_STOPONSHORTSOLUTIONS_FILTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with
 * STStopOnShortSolutionsFilter stipulation slice type.
 * Slices of this type make sure that solving stops after a short solution
 * solution has been found
 */

/* Allocate a STStopOnShortSolutionsFilter slice.
 * @param length full length
 * @param length minimum length
 * @return allocated slice
 */
slice_index alloc_stoponshortsolutions_filter(stip_length_type length,
                                              stip_length_type min_length);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type stoponshortsolutions_solve(slice_index si, stip_length_type n);

#endif
