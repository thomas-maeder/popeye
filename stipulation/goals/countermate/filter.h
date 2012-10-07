#if !defined(STIPULATION_GOALS_COUNTERMATE_FILTER_H)
#define STIPULATION_GOALS_COUNTERMATE_FILTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STCounterMateFilter stipulation slices.
 * Slices of this type make sure that the prerequisites for reaching a
 * countermate are fulfilled (i.e. that the side delivering the countermate
 * is mated before the move delivering it).
 */

/* Allocate a STCounterMateFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_countermate_filter_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type countermate_filter_solve(slice_index si, stip_length_type n);

#endif
