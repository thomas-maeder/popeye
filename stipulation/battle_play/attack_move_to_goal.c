#include "stipulation/battle_play/attack_move_to_goal.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/attack_fork.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STAttackMoveToGoal slice.
 * @param goal goal to be reached
 * @return index of allocated slice
 */
slice_index alloc_attack_move_to_goal_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_branch(STAttackMoveToGoal,
                        slack_length_battle+1,slack_length_battle);
  slices[result].u.branch.imminent_goal = goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Optimise a STAttackMove slice
 * @param si identifies slice to be optimised
 * @param goal goal that slice si attempts to reach
 */
void optimise_final_attack_move(slice_index si, Goal goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const fork = alloc_attack_fork_slice(length,min_length,proxy1);
    slice_index const last_attack = alloc_attack_move_to_goal_slice(goal);
    slice_index const proxy2 = alloc_proxy_slice();

    pipe_append(slices[si].prev,fork);

    pipe_link(proxy1,last_attack);
    pipe_link(last_attack,proxy2);
    pipe_set_successor(proxy2,slices[si].u.pipe.next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Iterate over the attacker's moves until a solution is found
 * @param si slice index of slice being solved
 * @return true iff a solution was found
 */
static boolean find_imminent_solution(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slack_length_battle+1;
  stip_length_type const n_max_unsolvable = slack_length_battle-1;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && defense_can_defend_in_n(next,n-1,n_max_unsolvable)<=n-1)
    {
      result = true;
      repcoup();
      break;
    }
    else
      repcoup();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution to the imminent goal
 * @param si slice identifier
 * @return true iff we have a solution
 */
static boolean have_we_solution_for_imminent_goal(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.branch.imminent_goal.type!=no_goal);

  move_generation_mode = move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  empile_for_goal = slices[si].u.branch.imminent_goal;
  generate_move_reaching_goal(slices[si].starter);
  empile_for_goal.type = no_goal;
  result = find_imminent_solution(si);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
attack_move_to_goal_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n==slack_length_battle+1);

  if (have_we_solution_for_imminent_goal(si))
    result = slack_length_battle+1;
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try each attacker's move as a solution
 * @param si slice index
 * @return true iff >=1 solution was found
 */
static boolean foreach_move_solve_imminent(slice_index si)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n = slack_length_battle+1;
  stip_length_type const n_max_unsolvable = slack_length_battle;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && defense_defend_in_n(next,n-1,n_max_unsolvable-1)<=n-1)
      result = true;

    repcoup();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find solutions for the imminent goal
 * @param si slice index
 * @return true iff >=1 solution was found
 */
static boolean solve_imminent_goal(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.branch.imminent_goal.type!=no_goal);

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  empile_for_goal = slices[si].u.branch.imminent_goal;
  generate_move_reaching_goal(slices[si].starter);
  empile_for_goal.type = no_goal;
  result = foreach_move_solve_imminent(si);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
attack_move_to_goal_solve_in_n(slice_index si,
                               stip_length_type n,
                               stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n==slack_length_battle+1);

  if (solve_imminent_goal(si))
    result = slack_length_battle+1;
  else
    result = slack_length_battle+3;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
