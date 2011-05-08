#include "stipulation/goals/reached_tester.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/leaf.h"
#include "trace.h"

/* Allocate a STGoalReachedTesting slice
 * @return identifier of the allocated slice
 */
slice_index alloc_goal_testing_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalReachedTesting);
  slices[result].u.goal_tester.fork = alloc_proxy_slice();
  pipe_link(slices[result].u.goal_tester.fork,alloc_leaf_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_testing_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_solve(slices[si].u.goal_tester.fork);
  if (result==has_solution)
    result = slice_solve(slices[si].u.goal_tester.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_testing_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.goal_tester.fork);
  if (result==has_solution)
    result = slice_has_solution(slices[si].u.goal_tester.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
