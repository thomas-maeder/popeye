#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "stipulation/boolean/and.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/notcheck/reached_tester.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality that detects whether double stalemate goal
 * has been reached
 */

/* Allocate a system of slices that test whether double stalemate has
 * been reached
 * @return index of entry slice
 */
slice_index alloc_goal_doublestalemate_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Goal const goal = { goal_dblstale, initsquare };
    slice_index const dblstale_tester = alloc_pipe(STGoalDoubleStalemateReachedTester);
    slice_index const proxy_starter = alloc_proxy_slice();
    slice_index const proxy_other = alloc_proxy_slice();
    slice_index const and_index = alloc_and_slice(proxy_other,proxy_starter);
    slice_index const notcheck_tester = alloc_goal_notcheck_reached_tester_slice();
    slice_index const immobile_tester_starter = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
    slice_index const immobile_tester_other = alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary);

    pipe_link(dblstale_tester,and_index);

    pipe_link(proxy_starter,notcheck_tester);
    pipe_link(notcheck_tester,immobile_tester_starter);
    pipe_link(immobile_tester_starter,alloc_true_slice());

    pipe_link(proxy_other,immobile_tester_other);
    pipe_link(immobile_tester_other,alloc_true_slice());

    result = alloc_goal_reached_tester_slice(goal,dblstale_tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
