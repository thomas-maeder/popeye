#include "stipulation/goals/castling/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/reached_tester.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a castling goal has just been reached
 */

/* Allocate a system of slices that tests whether castling has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_castling_reached_tester_system(void)
{
  slice_index result;
  slice_index castling_tester;
  Goal const goal = { goal_castling, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  castling_tester = alloc_pipe(STGoalCastlingReachedTester);
  pipe_link(castling_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,castling_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
