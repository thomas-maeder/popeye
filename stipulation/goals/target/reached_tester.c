#include "stipulation/goals/target/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a goal has just been reached
 */

/* Allocate a system of slices that tests whether a target square has been
 * reached
 * @return index of entry slice
 */
slice_index alloc_goal_target_reached_tester_system(square target)
{
  slice_index result;
  slice_index target_tester;
  Goal const goal = { goal_target, target };

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParamListEnd();

  target_tester = alloc_pipe(STGoalTargetReachedTester);
  slices[target_tester].u.goal_handler.goal.type = goal_target;
  slices[target_tester].u.goal_handler.goal.target = target;
  pipe_link(target_tester,alloc_true_slice());

  result = alloc_goal_reached_tester_slice(goal,target_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
