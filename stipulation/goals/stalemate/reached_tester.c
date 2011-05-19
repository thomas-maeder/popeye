#include "stipulation/goals/stalemate/reached_tester.h"
#include "pypipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/notcheck/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

/* This module provides functionality that detects whether stalemate goal has
 * been reached
 */

/* Allocate a system of slices that tests whether mate has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_stalemate_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Goal const goal = { goal_stale, initsquare };
    slice_index const stalemate_tester = alloc_pipe(STGoalStalemateReachedTester);
    slice_index const notcheck_tester = alloc_goal_notcheck_reached_tester_slice();
    slice_index const immobile_tester = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);

    pipe_link(stalemate_tester,notcheck_tester);
    pipe_link(notcheck_tester,immobile_tester);
    pipe_link(immobile_tester,alloc_true_slice());

    result = alloc_goal_reached_tester_slice(goal,stalemate_tester);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
