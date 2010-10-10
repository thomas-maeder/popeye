#include "stipulation/goals/atob/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyproof.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether an A=>B game goal has just been reached
 */

/* Allocate a STGoalAToBReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_atob_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalAToBReachedTester);
  slices[result].u.goal_reached_tester.goal.type = goal_atob;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* otherwise, we are reusing the proof game functionality */
