#include "stipulation/goals/reached_tester.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goal_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (solve(slices[si].next2,slack_length))
  {
    case previous_move_is_illegal:
      result = previous_move_is_illegal;
      break;

    case slack_length+2:
      result = n+2;
      break;

    case slack_length:
      result = solve(slices[si].next1,n);
      break;

    default:
      assert(0);
      result = previous_move_is_illegal;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
