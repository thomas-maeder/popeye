#include "stipulation/goals/notcheck/reached_tester.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Allocate a STGoalNotCheckReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_notcheck_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalNotCheckReachedTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
