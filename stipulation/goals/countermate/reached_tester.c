#include "stipulation/goals/countermate/reached_tester.h"
#include "pypipe.h"
#include "trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a counter mate goal has just been reached
 */

/* Allocate a STGoalCounterMateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_countermate_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalCounterMateReachedTester);
  slices[result].u.goal_reached_tester.goal.type = goal_countermate;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* otherwise, we are reusing the double mate functionality */
