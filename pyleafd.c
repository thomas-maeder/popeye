#include "pyleafd.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyoutput.h"
#include "pyleaf.h"
#include "pyoutput.h"
#include "pymsg.h"

#include <assert.h>
#include <stdlib.h>

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param leaf slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean leaf_d_are_threats_refuted(table threats, slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (table_length(threats)>0)
  {
    unsigned int nr_successful_threats = 0;
    boolean defense_found = false;

    generate_move_reaching_goal(leaf,attacker);

    while (encore() && !defense_found)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && is_current_move_in_table(threats))
      {
        if (leaf_is_goal_reached(attacker,leaf)==goal_reached)
          ++nr_successful_threats;
        else
          defense_found = true;
      }

      repcoup();
    }

    finply();

    /* this happens if >=1 threat no longer works or some threats can
     * no longer be played after the defense.
     */
    result = nr_successful_threats<table_length(threats);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in a direct leaf.
 * @param leaf slice index of leaf slice
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_d_has_solution(slice_index leaf)
{
  has_solution_type result = has_no_solution;
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (slices[leaf].u.leaf.goal==goal_doublemate
      && immobile(attacker))
    TraceText("attacker is immobile\n");
  else
  {
    generate_move_reaching_goal(leaf,attacker);

    while (encore() && result==has_no_solution)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(attacker,leaf)==goal_reached)
      {
        result = has_solution;
        coupfort();
      }

      repcoup();

      if (periods_counter>=nr_periods)
        break;
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write keys leading to a double-mate
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_root_dmate_solve(slice_index leaf)
{
  Side const starter = slices[leaf].starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[leaf].starter)))
    ErrorMsg(KingCapture);
  else if (!immobile(starter))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    solutions = 0;

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf)==goal_reached)
      {
        result = true;
        write_final_attack(goal_doublemate,attack_key);
        output_start_leaf_variation_level();
        output_end_leaf_variation_level();
        write_end_of_solution();
      }

      repcoup();

      if (OptFlag[maxsols] && solutions>=maxsolutions)
      {
        /* signal maximal number of solutions reached to outer world */
        FlagMaxSolsReached = true;
        break;
      }
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return true;
}

/* Determine and write keys leading to counter-mate
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_root_cmate_solve(slice_index leaf)
{
  Side const starter = slices[leaf].starter;
  Side const non_starter = advers(starter);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  /* TODO can this be generalised to non-mate goals? */
  if (goal_checker_mate(non_starter)==goal_reached)
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    solutions = 0;

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf)==goal_reached)
      {
        result = true;
        write_final_attack(goal_countermate,attack_key);
        output_start_leaf_variation_level();
        output_end_leaf_variation_level();
        write_end_of_solution();
      }

      repcoup();

      if (OptFlag[maxsols] && solutions>=maxsolutions)
      {
        /* signal maximal number of solutions reached to outer world */
        FlagMaxSolsReached = true;
        break;
      }
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write keys leading to "regular goals"
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_root_regulargoals_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[leaf].starter)))
    ErrorMsg(KingCapture);
  else
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,attacker);

    solutions = 0;

    while (encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(attacker,leaf)==goal_reached)
      {
        result = true;
        write_final_attack(slices[leaf].u.leaf.goal,attack_key);
        output_start_leaf_variation_level();
        output_end_leaf_variation_level();
        write_end_of_solution();
      }

      repcoup();

      if (OptFlag[maxsols] && solutions>=maxsolutions)
      {
        /* signal maximal number of solutions reached to outer world */
        FlagMaxSolsReached = true;
        break;
      }
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write keys at root level
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
boolean leaf_d_root_solve(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  init_output(leaf);

  output_start_continuation_level();

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_countermate:
      result = leaf_d_root_cmate_solve(leaf);
      break;

    case goal_doublemate:
      result = leaf_d_root_dmate_solve(leaf);
      break;

    default:
      result = leaf_d_root_regulargoals_solve(leaf);
      break;
  }

  output_end_continuation_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

enum
{
  leaf_has_solution = slack_length_direct+1,
  leaf_has_no_solution = slack_length_direct+3
};

/* Determine and write keys leading to a double-mate
 * @param leaf leaf's slice index
 * @return leaf_has_solution if >=1 key was found and written
 *         leaf_has_no_solution otherwise
 */
static stip_length_type leaf_d_dmate_solve(slice_index leaf)
{
  stip_length_type result = leaf_has_no_solution;
  Side const starter = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (!immobile(starter))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf)==goal_reached)
      {
        result = leaf_has_solution;
        write_final_attack(goal_doublemate,attack_key);
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

/* Determine and write keys leading to counter-mate
 * @param leaf leaf's slice index
 * @return leaf_has_solution if >=1 key was found and written
 *         leaf_has_no_solution otherwise
 */
static stip_length_type leaf_d_cmate_solve(slice_index leaf)
{
  stip_length_type result = leaf_has_no_solution;
  Side const starter = slices[leaf].starter;
  Side const non_starter = advers(starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  /* TODO can this be generalised to non-mate goals? */
  if (goal_checker_mate(non_starter)==goal_reached)
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf)==goal_reached)
      {
        result = leaf_has_solution;
        write_final_attack(goal_countermate,attack_key);
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

/* Determine and write keys leading to "regular goals"
 * @param leaf leaf's slice index
 * @return leaf_has_solution if >=1 key was found and written
 *         leaf_has_no_solution otherwise
 */
static stip_length_type leaf_d_regulargoals_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  stip_length_type result = leaf_has_no_solution;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && leaf_is_goal_reached(attacker,leaf)==goal_reached)
    {
      result = leaf_has_solution;
      write_final_attack(slices[leaf].u.leaf.goal,attack_key);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type leaf_d_solve_in_n(slice_index leaf,
                                   stip_length_type n,
                                   stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n==slack_length_direct+1);
  assert(n_min==slack_length_direct+1);

  output_start_continuation_level();

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_countermate:
      result = leaf_d_cmate_solve(leaf);
      break;

    case goal_doublemate:
      result = leaf_d_dmate_solve(leaf);
      break;

    default:
      result = leaf_d_regulargoals_solve(leaf);
      break;
  }

  output_end_continuation_level();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean leaf_d_defend(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  if (leaf_d_has_solution(leaf)==has_no_solution)
    result = true;
  else
  {
    result = false;
    write_attack(attack_regular);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param leaf index of branch slice
 */
void leaf_d_solve_threats(table threats, slice_index leaf)
{
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && leaf_is_goal_reached(attacker,leaf)==goal_reached)
    {
      write_final_attack(slices[leaf].u.leaf.goal,attack_regular);
      output_start_leaf_variation_level();
      output_end_leaf_variation_level();
      append_to_top_table();
      coupfort();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean leaf_d_detect_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  stip_detect_starter_param_type * const param = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  param->who_decides = leaf_decides_on_starter;
  if (slices[si].starter==no_side)
    slices[si].starter = White;

  TraceEnumerator(Side,slices[si].starter,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
