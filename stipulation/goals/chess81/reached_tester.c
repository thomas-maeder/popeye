#include "stipulation/goals/chess81/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether an chess81 goal has just been reached
 */

/* Allocate a system of slices that tests whether first row or last row has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_chess81_reached_tester_system(void)
{
  slice_index result;
  slice_index chess81_tester;
  Goal const goal = { goal_chess81, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  chess81_tester = alloc_pipe(STGoalChess81ReachedTester);
  pipe_link(chess81_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,chess81_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
