#include "stipulation/battle_play/attack_move.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "pypipe.h"
#include "pyleaf.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_root.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STAttackMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_move_slice(stip_length_type length,
                                    stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  assert((length-slack_length_battle)%2==(min_length-slack_length_battle)%2);

  result = alloc_branch(STAttackMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void attack_move_insert_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index attack_root;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.pipe.next,st);

  attack_root = alloc_attack_root_slice(length,min_length);
  pipe_link(attack_root,*root);
  *root = attack_root;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Iterate over the attacker's moves until a solution is found
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return true iff a solution was found
 */
static boolean find_solution(slice_index si,
                             stip_length_type n,
                             stip_length_type n_max_unsolvable)
{
  boolean result = false;
  unsigned int const nr_refutations_allowed = 0;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && defense_can_defend_in_n(next,
                                   n-1,n_max_unsolvable-1,
                                   nr_refutations_allowed)<n)
    {
      result = true;
      coupfort();
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

/* Determine whether this slice has a solution in n half moves
 * @param si slice identifier
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return true iff the attacking side wins
 */
static boolean have_we_solution_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_max_unsolvable)
{
  boolean result;
  Side const attacker = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);

  move_generation_mode = move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  active_slice[nbply+1] = si;
  genmove(attacker);
  result = find_solution(si,n,n_max_unsolvable);
  finply();

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
  Side const attacker = slices[si].starter;
  Goal const imminent_goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (are_prerequisites_for_reaching_goal_met(imminent_goal,attacker))
  {
    stip_length_type const n = slack_length_battle+1;
    stip_length_type const n_max_unsolvable = slack_length_battle-1;
    move_generation_mode = move_generation_optimized_by_killer_move;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = si;
    empile_for_goal = imminent_goal;
    empile_for_target = slices[si].u.branch.imminent_target;
    generate_move_reaching_goal(attacker);
    empile_for_goal = no_goal;
    result = find_solution(si,n,n_max_unsolvable);
    finply();
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether attacker can end in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_move_has_solution_in_n(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_min)
{
  stip_length_type result;
  stip_length_type n_max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);
  assert(n_min>=slack_length_battle);

  n_max_unsolvable = battle_branch_calc_n_min(si,n)-2;

  if (n_min==slack_length_battle)
    n_min = slack_length_battle+2;
  else if (n_min==slack_length_battle+1)
  {
    if (have_we_solution_for_imminent_goal(si))
      /* no need to try to determine whether there is a longer solution */
      n = n_min-2;
    else
    {
      n_max_unsolvable = n_min;
      n_min = slack_length_battle+3;
    }
  }

  for (result = n_min; result<=n; result += 2)
    if (have_we_solution_in_n(si,result,n_max_unsolvable))
      break;
    else
      n_max_unsolvable = result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try each attacker's move as a solution
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return true iff >=1 solution was found
 */
static boolean foreach_move_solve(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_min,
                                  stip_length_type n_max_unsolvable)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && defense_defend_in_n(next,n-1,n_min-1,n_max_unsolvable-1)<=n-1)
    {
      result = true;
      coupfort();
    }

    repcoup();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find long solutions (i.e. solutions where the play goes on in the
 * current branch)
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return true iff >=1 solution in n half-moves was found
 */
static boolean solve_in_n(slice_index si,
                          stip_length_type n,
                          stip_length_type n_max_unsolvable)
{
  boolean result;
  Side const attacker = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  active_slice[nbply+1] = si;
  genmove(attacker);
  result = foreach_move_solve(si,
                              n,battle_branch_calc_n_min(si,n),
                              n_max_unsolvable);
  finply();

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
  Side const attacker = slices[si].starter;
  Goal const imminent_goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (are_prerequisites_for_reaching_goal_met(imminent_goal,attacker))
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = si;
    empile_for_goal = imminent_goal;
    empile_for_target = slices[si].u.branch.imminent_target;
    generate_move_reaching_goal(attacker);
    empile_for_goal = no_goal;
    result = foreach_move_solve(si,
                                slack_length_battle+1,slack_length_battle+1,
                                slack_length_battle-1);
    finply();
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_move_solve_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min)
{
  stip_length_type result;
  stip_length_type n_max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);
  assert(n_min>=slack_length_battle);
  
  n_max_unsolvable = battle_branch_calc_n_min(si,n)-2;

  output_start_continuation_level(si);

  if (n_min==slack_length_battle)
    n_min = slack_length_battle+2;
  else if (n_min==slack_length_battle+1)
  {
    if (solve_imminent_goal(si))
      /* no need to try to find longer solutions */
      n = n_min-2;
    else
    {
      n_max_unsolvable = n_min;
      n_min = slack_length_battle+3;
    }
  }

  for (result = n_min; result<=n; result += 2)
    if (solve_in_n(si,result,n_max_unsolvable))
      break;
    else
      n_max_unsolvable = result;

  output_end_continuation_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void attack_move_detect_starter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    stip_traverse_structure(slices[si].u.pipe.next,st);
    slices[si].starter = advers(slices[slices[si].u.pipe.next].starter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void attack_move_apply_setplay(slice_index si, stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *setplay_slice = stip_make_setplay(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
