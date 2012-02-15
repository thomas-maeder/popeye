#include "stipulation/goals/capture/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a capture goal has just been reached
 */

/* Allocate a system of slices that tests whether capture has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_capture_reached_tester_system(void)
{
  slice_index result;
  slice_index capture_tester;
  Goal const goal = { goal_capture, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  capture_tester = alloc_pipe(STGoalCaptureReachedTester);
  pipe_link(capture_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,capture_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_capture_reached_tester_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbply!=nil_ply);

  if (pprise[nbply]!=vide)
    result = slice_solve(slices[si].u.pipe.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
