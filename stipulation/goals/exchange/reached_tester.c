#include "stipulation/goals/exchange/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether an exchange goal has just been reached
 */

/* Allocate a system of slices that tests whether exchange has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_exchange_reached_tester_system(void)
{
  slice_index result;
  slice_index exchange_tester;
  Goal const goal = { goal_exchange, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  exchange_tester = alloc_pipe(STGoalExchangeReachedTester);
  pipe_link(exchange_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,exchange_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
