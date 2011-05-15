#include "stipulation/goals/exchange_by_rebirth/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/reached_tester.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a exchange (by rebirth) goal has just been reached
 */

/* Allocate a system of slices that tests whether exchange_by_rebirth has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_exchange_by_rebirth_reached_tester_system(void)
{
  slice_index result;
  slice_index exchange_by_rebirth_tester;
  Goal const goal = { goal_exchange_by_rebirth, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  exchange_by_rebirth_tester = alloc_pipe(STGoalExchangeByRebirthReachedTester);
  pipe_link(exchange_by_rebirth_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,exchange_by_rebirth_tester);

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
has_solution_type
goal_exchange_by_rebirth_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  /* goal is only reachable in some fairy conditions */
  result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
goal_exchange_by_rebirth_reached_tester_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  /* goal is only reachable in some fairy conditions */
  result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
