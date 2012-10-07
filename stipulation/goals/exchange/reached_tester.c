#include "stipulation/goals/exchange/reached_tester.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goal_exchange_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  square const sq_diagram = GetPositionInDiagram(spec[sq_arrival]);
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(current_move[nbply]!=nil_coup);

  if (GetPositionInDiagram(spec[sq_diagram])==sq_arrival
      && (just_moved==White ? e[sq_diagram]>=roib : e[sq_diagram]<=roin)
      && sq_diagram!=sq_arrival)
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
