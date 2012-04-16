#include "stipulation/goals/atob/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyproof.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether an A=>B game goal has just been reached
 */

/* Allocate a system of slices that tests whether atob has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_atob_reached_tester_system(void)
{
  slice_index result;
  slice_index atob_tester;
  Goal const goal = { goal_atob, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  atob_tester = alloc_pipe(STGoalAToBReachedTester);
  pipe_link(atob_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,atob_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* otherwise, we are reusing the proof game functionality */
