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

  battle_branch_shorten_slice(si);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal (after the defense)
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean attack_move_are_threats_refuted_in_n(table threats,
                                             stip_length_type len_threat,
                                             slice_index si,
                                             stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int const nr_refutations_allowed = 0;
  boolean result;
  unsigned int nr_successful_threats = 0;
  boolean defense_found = false;
  stip_length_type n_min;
  Goal const imminent_goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);

  n_min = battle_branch_calc_n_min(si,len_threat);

  if (n_min==slack_length_battle+1
      && !are_prerequisites_for_reaching_goal_met(imminent_goal,attacker))
    n_min = slack_length_battle+3;

  if (n_min<=n)
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = si;
    if (n<=slack_length_battle+1 && imminent_goal!=no_goal)
    {
      empile_for_goal = imminent_goal;
      empile_for_target = slices[si].u.branch.imminent_target;
      generate_move_reaching_goal(attacker);
      empile_for_goal = no_goal;
    }
    else
      genmove(attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && is_current_move_in_table(threats))
      {
        stip_length_type const
            nr_moves_needed = defense_can_defend_in_n(next,
                                                      len_threat-1,n_min-1,
                                                      nr_refutations_allowed);
        if (nr_moves_needed<slack_length_battle || nr_moves_needed>=len_threat)
          defense_found = true;
        else
          ++nr_successful_threats;
      }

      repcoup();
    }

    finply();

    /* this happens if >=1 threat no longer works or some threats can
     * no longer be played after the defense.
     */
    result = nr_successful_threats<table_length(threats);
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether this slice has a solution in n half moves
 * @param si slice identifier
 * @param n maximum number of half moves until goal
 * @return true iff the attacking side wins
 */
static boolean have_we_solution_in_n(slice_index si,
                                     stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean solution_found = false;
  unsigned int const nr_refutations_allowed = 0;
  stip_length_type n_min;
  Goal const imminent_goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  n_min = battle_branch_calc_n_min(si,n);

  assert(n%2==slices[si].u.branch.length%2);

  if (n_min==slack_length_battle+1
      && !are_prerequisites_for_reaching_goal_met(imminent_goal,attacker))
    n_min = slack_length_battle+3;

  if (n_min<=n)
  {
    move_generation_mode = move_generation_optimized_by_killer_move;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = si;
    if (n<=slack_length_battle+1 && imminent_goal!=no_goal)
    {
      empile_for_goal = imminent_goal;
      empile_for_target = slices[si].u.branch.imminent_target;
      generate_move_reaching_goal(attacker);
      empile_for_goal = no_goal;
    }
    else
      genmove(attacker);

    while (!solution_found && encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      {
        stip_length_type const
            nr_moves_needed = defense_can_defend_in_n(next,
                                                      n-1,n_min-1,
                                                      nr_refutations_allowed);
        if (n_min-1<=nr_moves_needed && nr_moves_needed<n)
        {
          solution_found = true;
          coupfort();
        }
      }

      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",n);
  TraceFunctionResult("%u",solution_found);
  TraceFunctionResultEnd();
  return solution_found;
}

/* Determine whether attacker can end in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            n_min-4 defense has turned out to be illegal
 *            n_min-2 defense has solved
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_move_has_solution_in_n(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (n_min<=slack_length_battle)
    n_min += 2;

  for (result = n_min; result<=n; result += 2)
    if (have_we_solution_in_n(si,result))
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve long threats (i.e. threats where the play goes on in the
 * current branch)
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff >=1 threat was found
 */
static boolean solve_threats_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type n_min;
  Goal const imminent_goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  n_min = battle_branch_calc_n_min(si,n);

  if (n_min==slack_length_battle+1
      && !are_prerequisites_for_reaching_goal_met(imminent_goal,attacker))
    n_min = slack_length_battle+3;

  if (n_min<=n)
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = si;
    if (n<=slack_length_battle+1 && imminent_goal!=no_goal)
    {
      empile_for_goal = imminent_goal;
      empile_for_target = slices[si].u.branch.imminent_target;
      generate_move_reaching_goal(attacker);
      empile_for_goal = no_goal;
    }
    else
      genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !defense_defend_in_n(next,n-1,n_min-1))
      {
        result = true;
        coupfort();
        append_to_top_table();
      }

      repcoup();
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type attack_move_solve_threats_in_n(table threats,
                                                slice_index si,
                                                stip_length_type n,
                                                stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);

  if (n_min<=slack_length_battle)
    n_min += 2;

  for (result = n_min; result<=n; result += 2)
    if (solve_threats_in_n(si,result))
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find long solutions (i.e. solutions where the play goes on in the
 * current branch)
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return true iff >=1 solution in n half-moves was found
 */
static boolean solve_in_n(slice_index si, stip_length_type n)
{
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;
  boolean result = false;
  stip_length_type n_min;
  Goal const imminent_goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  n_min = battle_branch_calc_n_min(si,n);

  if (n_min==slack_length_battle+1
      && !are_prerequisites_for_reaching_goal_met(imminent_goal,attacker))
    n_min = slack_length_battle+3;

  if (n_min<=n)
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    active_slice[nbply+1] = si;
    if (n<=slack_length_battle+1 && imminent_goal!=no_goal)
    {
      empile_for_goal = imminent_goal;
      empile_for_target = slices[si].u.branch.imminent_target;
      generate_move_reaching_goal(attacker);
      empile_for_goal = no_goal;
    }
    else
      genmove(attacker);

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !defense_defend_in_n(next,n-1,n_min-1))
      {
        result = true;
        coupfort();
      }

      repcoup();
    }

    finply();
  }

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
 *            n_min-4 defense has turned out to be illegal
 *            n_min-2 defense has solved
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_move_solve_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.branch.length%2);

  output_start_continuation_level();

  if (n_min<=slack_length_battle)
    n_min += 2;

  for (result = n_min; result<=n; result += 2)
    if (solve_in_n(si,result))
      break;

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
