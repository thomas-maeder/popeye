#include "stipulation/goals/mate/reached_tester.h"
#include "pypipe.h"
#include "trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a mate goal has just been reached
 */

/* Allocate a STGoalMateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_mate_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalMateReachedTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
