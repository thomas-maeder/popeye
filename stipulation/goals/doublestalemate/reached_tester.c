#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a double stalemate goal has just been reached
 */

/* Allocate a STGoalDoubleStalemateReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_doublestalemate_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalDoubleStalemateReachedTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
