#include "stipulation/goals/target/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

#include <assert.h>

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
  slices[target_tester].u.goal_target_reached_tester.target = target;
  pipe_link(target_tester,alloc_true_slice());

  result = alloc_goal_reached_tester_slice(goal,target_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_target_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  square const target = slices[si].u.goal_target_reached_tester.target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  if (move_generation_stack[nbcou].arrival==target)
    result = slice_solve(slices[si].u.goal_target_reached_tester.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
