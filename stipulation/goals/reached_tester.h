#if !defined(STIPULATION_GOALS_REACHED_TESTER_H)
#define STIPULATION_GOALS_REACHED_TESTER_H

#include "stipulation/goals/goals.h"
#include "stipulation/stipulation.h"

/* Allocate a STGoalReachedTester slice
 * @param goal goal to be tested
 * @param tester identifies the slice(s) that actually tests
 * @return identifier of the allocated slice
 */
slice_index alloc_goal_reached_tester_slice(Goal goal, slice_index tester);

#endif
