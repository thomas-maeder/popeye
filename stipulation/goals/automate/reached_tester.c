#include "stipulation/goals/automate/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a auto-mate goal has just been reached
 */

/* Allocate a system of slices that tests whether automate has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_automate_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Goal const goal = { goal_automate, initsquare };
    slice_index const automate_tester = alloc_pipe(STGoalAutoMateReachedTester);
    slice_index const notcheck_tester = alloc_goal_check_reached_tester_slice(goal_applies_to_adversary);
    slice_index const immobile_tester = alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary);

    pipe_link(automate_tester,notcheck_tester);
    pipe_link(notcheck_tester,immobile_tester);
    pipe_link(immobile_tester,alloc_true_slice());

    result = alloc_goal_reached_tester_slice(goal,automate_tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
