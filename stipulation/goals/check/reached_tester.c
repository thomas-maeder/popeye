#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a check goal has just been reached
 */

/* Allocate a system of slices that tests whether check has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_check_reached_tester_system(void)
{
  slice_index result;
  slice_index check_tester;
  Goal const goal = { goal_check, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  check_tester = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
  pipe_link(check_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,check_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STGoalCheckReachedTester slice.
 * @param starter_or_adversary is the starter or its adversary to be in check?
 * @return index of allocated slice
 */
slice_index alloc_goal_check_reached_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalCheckReachedTester);
  slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
