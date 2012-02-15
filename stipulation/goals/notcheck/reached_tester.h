#if !defined(STIPULATION_GOAL_NONCHECK_REACHED_TESTER_H)
#define STIPULATION_GOAL_NONCHECK_REACHED_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is not in check
 */

/* Allocate a STGoalNotCheckReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_notcheck_reached_tester_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_notcheck_reached_tester_solve(slice_index si);

#endif
