#if !defined(STIPULATION_GOALS_REACHED_TESTER_H)
#define STIPULATION_GOALS_REACHED_TESTER_H

#include "pyslice.h"
#include "stipulation/goals/goals.h"

/* Allocate a STGoalReachedTester slice
 * @param goal goal to be tested
 * @param tester identifies the slice(s) that actually tests
 * @return identifier of the allocated slice
 */
slice_index alloc_goal_reached_tester_slice(Goal goal, slice_index tester);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_tester_solve(slice_index si);

#endif
