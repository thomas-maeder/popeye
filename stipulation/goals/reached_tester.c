#include "stipulation/goals/reached_tester.h"
#include "pystip.h"
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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goal_reached_tester_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (attack(slices[si].next2,length_unspecified))
  {
    case opponent_self_check:
      result = slack_length-2;
      break;

    case has_no_solution:
      result = n+2;
      break;

    case has_solution:
      result = attack(slices[si].next1,n);
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type goal_reached_tester_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (attack(slices[si].next2,length_unspecified))
  {
    case opponent_self_check:
      result = slack_length-2;
      break;

    case has_no_solution:
      result = n+2;
      break;

    case has_solution:
      result = defend(slices[si].next1,n);
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
