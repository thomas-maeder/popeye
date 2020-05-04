#include "stipulation/goals/countermate/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "stipulation/boolean/and.h"
#include "debugging/trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a counter mate goal has just been reached
 */

/* Allocate a system of slices that tests whether countermate has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_countermate_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Goal const goal = { goal_countermate, initsquare };
    slice_index const countermate_tester = alloc_pipe(STGoalCounterMateReachedTester);
    slice_index const proxy_starter = alloc_proxy_slice();
    slice_index const proxy_other = alloc_proxy_slice();
    slice_index const and_index = alloc_and_slice(proxy_other,proxy_starter);
    slice_index const check_tester_starter = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
    slice_index const check_tester_other = alloc_goal_check_reached_tester_slice(goal_applies_to_adversary);
    slice_index const immobile_tester_starter = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
    slice_index const immobile_tester_other = alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary);

    pipe_link(countermate_tester,and_index);

    pipe_link(proxy_starter,check_tester_starter);
    pipe_link(check_tester_starter,immobile_tester_starter);
    pipe_link(immobile_tester_starter,alloc_true_slice());

    pipe_link(proxy_other,check_tester_other);
    pipe_link(check_tester_other,immobile_tester_other);
    pipe_link(immobile_tester_other,alloc_true_slice());

    result = alloc_goal_reached_tester_slice(goal,countermate_tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
