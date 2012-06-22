#include "stipulation/goals/mate/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

/* This module provides functionality that detects whether mate goal has
 * been reached
 */

/* Allocate a system of slices that tests whether mate has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_mate_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const mate_tester = alloc_pipe(STGoalMateReachedTester);
    slice_index const check_tester = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
    slice_index const immobile_tester = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);

    pipe_link(mate_tester,check_tester);
    pipe_link(check_tester,immobile_tester);
    pipe_link(immobile_tester,alloc_true_slice());

    result = mate_tester;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
