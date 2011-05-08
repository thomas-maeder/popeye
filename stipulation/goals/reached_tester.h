#if !defined(STIPULATION_GOALS_REACHED_TESTER_H)
#define STIPULATION_GOALS_REACHED_TESTER_H

#include "pyslice.h"

/* Allocate a STGoalReachedTester slice
 * @return identifier of the allocated slice
 */
slice_index alloc_goal_testing_slice(void);
/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_testing_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_testing_has_solution(slice_index si);

#endif
