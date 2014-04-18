#include "stipulation/goals/reached_tester.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STGoalReachedTester slice
 * @param goal goal to be tested
 * @param tester identifies the slice(s) that actually tests
 * @return identifier of the allocated slice
 */
slice_index alloc_goal_reached_tester_slice(Goal goal, slice_index tester)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParam("%u",tester);
  TraceFunctionParamListEnd();

  result = alloc_conditional_pipe(STGoalReachedTester,alloc_proxy_slice());
  slices[result].u.goal_handler.goal = goal;
  pipe_link(slices[result].next2,tester);

  pipe_link(result,alloc_true_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
