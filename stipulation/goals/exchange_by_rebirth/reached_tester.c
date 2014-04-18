#include "stipulation/goals/exchange_by_rebirth/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/reached_tester.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
