#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "stipulation/boolean/and.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a double mate goal has just been reached
 */

/* Allocate a system of slices that tests whether doublemate has been reached
 * @return index of entry slice
 */
slice_index alloc_doublemate_mate_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Goal const goal = { goal_doublemate, initsquare };
    slice_index const doublemate_tester = alloc_pipe(STGoalDoubleMateReachedTester);
    slice_index const proxy_starter = alloc_proxy_slice();
    slice_index const proxy_other = alloc_proxy_slice();
    slice_index const and_index = alloc_and_slice(proxy_other,proxy_starter);
    slice_index const check_tester_starter = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
    slice_index const check_tester_other = alloc_goal_check_reached_tester_slice(goal_applies_to_adversary);
    slice_index const immobile_tester_starter = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
    slice_index const immobile_tester_other = alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary);

    pipe_link(doublemate_tester,and_index);

    pipe_link(proxy_starter,check_tester_starter);
    pipe_link(check_tester_starter,immobile_tester_starter);
    pipe_link(immobile_tester_starter,alloc_true_slice());

    pipe_link(proxy_other,check_tester_other);
    pipe_link(check_tester_other,immobile_tester_other);
    pipe_link(immobile_tester_other,alloc_true_slice());

    result = alloc_goal_reached_tester_slice(goal,doublemate_tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
