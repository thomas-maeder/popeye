#include "stipulation/goal_reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a goal has just been reached
 */

/* Determine whether a side has reached the goal of a leaf slice.
 * @param camp side
 * @param goal goal to be tested
 * @return whether camp has reached leaf's goal
 */
goal_checker_result_type is_goal_reached(Side just_moved, Goal goal)
{
  boolean result = goal_not_reached;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,just_moved,"");
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_mate:
      /* still needed for exclusive chess and republican chess */
      result = goal_checker_mate(just_moved);
      break;

    case goal_stale:
    case goal_target:
    case goal_check:
    case goal_capture:
    case goal_steingewinn:
    case goal_ep:
    case goal_doublemate:
    case goal_countermate:
    case goal_castling:
    case goal_autostale:
    case goal_circuit:
    case goal_dblstale:
    case goal_exchange:
      assert(0);
      break;

    case goal_circuitB:
      result = goal_checker_circuitB(just_moved);
      break;

    case goal_exchangeB:
      result = goal_checker_exchangeB(just_moved);
      break;

    case goal_any:
      result = goal_checker_any(just_moved);
      break;

    case goal_proof:
    case goal_atob:
      result = goal_checker_proof(just_moved);
      break;

    case goal_mate_or_stale:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(goal_checker_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STGoalReachedTester slice.
 * @param type goal to be tested
 * @return index of allocated slice
 */
slice_index alloc_goal_reached_tester_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalReachedTester);
  slices[result].u.goal_reached_tester.goal = goal;

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
has_solution_type goal_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  Side const attacker = advers(slices[si].starter);
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (is_goal_reached(attacker,goal))
  {
    case goal_not_reached_selfcheck:
      result = opponent_self_check;
      break;

    case goal_not_reached:
      result = has_no_solution;
      break;

    case goal_reached:
      result = slice_has_solution(next);
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  Side const attacker = advers(slices[si].starter);
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (is_goal_reached(attacker,goal))
  {
    case goal_not_reached_selfcheck:
      result = opponent_self_check;
      break;

    case goal_not_reached:
      result = has_no_solution;
      break;

    case goal_reached:
      result = slice_solve(next);
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
goal_reached_tester_defend_in_n(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const attacker = advers(slices[si].starter);
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (is_goal_reached(attacker,goal))
  {
    case goal_not_reached_selfcheck:
    case goal_not_reached:
      result = n+4;
      break;

    case goal_reached:
      result = defense_defend_in_n(next,n,n_max_unsolvable);
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

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
goal_reached_tester_can_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const attacker = advers(slices[si].starter);
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (is_goal_reached(attacker,goal))
  {
    case goal_not_reached_selfcheck:
    case goal_not_reached:
      result = n+4;
      break;

    case goal_reached:
      result = defense_can_defend_in_n(next,n,n_max_unsolvable);
      break;

    default:
      assert(0);
      result = n+4;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
goal_reached_tester_solve_in_n(slice_index si,
                               stip_length_type n,
                               stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const attacker = advers(slices[si].starter);
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (is_goal_reached(attacker,goal))
  {
    case goal_not_reached_selfcheck:
      result = slack_length_battle-2;
      break;

    case goal_not_reached:
      result = n+2;
      break;

    case goal_reached:
      result = attack_solve_in_n(next,n,n_max_unsolvable);
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
goal_reached_tester_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const attacker = advers(slices[si].starter);
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  switch (is_goal_reached(attacker,goal))
  {
    case goal_not_reached_selfcheck:
      result = slack_length_battle-2;
      break;

    case goal_not_reached:
      result = n+2;
      break;

    case goal_reached:
      result = attack_has_solution_in_n(next,n,n_max_unsolvable);
      break;

    default:
      assert(0);
      result = n+2;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
