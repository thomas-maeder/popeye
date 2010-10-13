#include "stipulation/goals/any/reached_tester.h"
#include "pypipe.h"
#include "trace.h"

/* This module provides functionality dealing with slices that detect
 * whether an any goal has just been reached
 */

/* Allocate a STGoalAnyReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_any_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalAnyReachedTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
