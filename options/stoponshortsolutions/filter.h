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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type stoponshortsolutions_solve(slice_index si, stip_length_type n);

#endif
