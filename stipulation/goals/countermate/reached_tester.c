#include "stipulation/goals/countermate/reached_tester.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "stipulation/boolean/and.h"
#include "solving/king_capture_avoider.h"
#include "debugging/trace.h"

/* This module provides functionality dealing with slices that detect
 * whether a counter mate goal has just been reached
 */

/* Allocate a system of slices that tests whether countermate has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_countermate_reached_tester_system(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Goal const goal = { goal_countermate, initsquare };
    slice_index const countermate_tester = alloc_pipe(STGoalCounterMateReachedTester);
    slice_index const proxy_starter = alloc_proxy_slice();
    slice_index const proxy_other = alloc_proxy_slice();
    slice_index const and = alloc_and_slice(proxy_other,proxy_starter);
    slice_index const check_tester_starter = alloc_goal_check_reached_tester_slice(goal_applies_to_starter);
    slice_index const check_tester_other = alloc_goal_check_reached_tester_slice(goal_applies_to_adversary);
    slice_index const immobile_tester_starter = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
    slice_index const immobile_tester_other = alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary);

    pipe_link(countermate_tester,and);

    pipe_link(proxy_starter,check_tester_starter);
    pipe_link(check_tester_starter,immobile_tester_starter);
    pipe_link(immobile_tester_starter,alloc_true_slice());

    pipe_link(proxy_other,check_tester_other);
    pipe_link(check_tester_other,immobile_tester_other);
    pipe_link(immobile_tester_other,alloc_true_slice());

    result = alloc_goal_reached_tester_slice(goal,countermate_tester);
  }

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
stip_length_type goal_countermate_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u\n",parent_ply[nbply]);
  if (TSTFLAG(goal_preprequisites_met[parent_ply[nbply]],goal_countermate))
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
