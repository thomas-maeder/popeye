#if !defined(STIPULATION_ULTRASCHACHZWANG_GOAL_FILTER_H)
#define STIPULATION_ULTRASCHACHZWANG_GOAL_FILTER_H

#include "solving/solve.h"

/* This module provides slice type STUltraschachzwangGoalFilter. This slice
 * suspends Ultraschachzwang when testing for mate.
 */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ultraschachzwang_goal_filter_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation with Ultraschachzwang mate filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_ultraschachzwang_goal_filters(slice_index si);

#endif
