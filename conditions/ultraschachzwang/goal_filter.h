#if !defined(STIPULATION_ULTRASCHACHZWANG_GOAL_FILTER_H)
#define STIPULATION_ULTRASCHACHZWANG_GOAL_FILTER_H

#include "pyslice.h"

/* This module provides slice type STUltraschachzwangGoalFilter. This slice
 * suspends Ultraschachzwang when testing for mate.
 */

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ultraschachzwang_goal_filter_solve(slice_index si);

/* Instrument a stipulation with Ultraschachzwang mate filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_ultraschachzwang_goal_filters(slice_index si);

#endif
