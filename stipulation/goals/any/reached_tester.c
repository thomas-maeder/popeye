#include "stipulation/goals/any/reached_tester.h"
#include "pypipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

/* This module provides functionality dealing with slices that detect
 * whether an any goal has just been reached
 */

/* Allocate a system of slices that tests whether any has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_any_reached_tester_system(void)
{
  slice_index result;
  slice_index any_tester;
  Goal const goal = { goal_any, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  any_tester = alloc_pipe(STGoalAnyReachedTester);
  pipe_link(any_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,any_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_any_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_any_reached_tester_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_solve(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
