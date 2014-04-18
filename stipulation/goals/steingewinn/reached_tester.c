#include "stipulation/goals/steingewinn/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a steingewinn goal has just been reached
 */

/* Allocate a system of slices that tests whether steingewinn has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_steingewinn_reached_tester_system(void)
{
  slice_index result;
  slice_index steingewinn_tester;
  Goal const goal = { goal_steingewinn, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  steingewinn_tester = alloc_pipe(STGoalSteingewinnReachedTester);
  pipe_link(steingewinn_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,steingewinn_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
