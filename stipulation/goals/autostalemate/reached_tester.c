#include "stipulation/goals/autostalemate/reached_tester.h"
#include "pypipe.h"
#include "trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a auto-stalemate goal has just been reached
 */

/* Allocate a STGoalAutoStalemateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_autostalemate_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalAutoStalemateReachedTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
