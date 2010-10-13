#include "stipulation/goals/stalemate/reached_tester.h"
#include "pypipe.h"
#include "trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a stalemate goal has just been reached
 */

/* Allocate a STGoalStalemateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_stalemate_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalStalemateReachedTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
