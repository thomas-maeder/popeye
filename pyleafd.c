#include "pyleafd.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyoutput.h"
#include "pyleaf.h"
#include "pyhash.h"
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
        if (leaf_d_has_starter_won(leaf)==starter_has_won)
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
  hashwhat hash_value = nr_hashwhat;
  Side const attacker = slices[leaf].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  /* In move orientated stipulations (%, z, x etc.) it's less
   * expensive to compute an end in 1. TLi
   */
  if (!FlagMoveOrientatedStip)
  {
    /* It is more likely that a position has no solution. */
    /*    Therefore let's check for "no solution" first. TLi */
    if (inhash(leaf,DirNoSucc,1))
    {
      assert(!inhash(leaf,DirSucc,0));
      hash_value = DirNoSucc;
    }
    else if (inhash(leaf,DirSucc,0))
      hash_value = DirSucc;
  }

  if (hash_value==nr_hashwhat)
  {
    if (slices[leaf].u.leaf.goal==goal_doublemate
        && immobile(attacker))
    {
      TraceText("attacker is immobile\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%u\n",has_no_solution);
      return false;
    }

    generate_move_reaching_goal(leaf,attacker);

    while (encore() && hash_value!=DirSucc)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(attacker,leaf)==goal_reached)
      {
        hash_value = DirSucc;
        coupfort();
      }

      repcoup();

      if (periods_counter>=nr_periods)
        break;
    }

    finply();

    if (!FlagMoveOrientatedStip)
    {
      if (hash_value==DirSucc)
        addtohash(leaf,DirSucc,0);
      else
        addtohash(leaf,DirNoSucc,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,
                  hash_value==DirSucc ? has_solution : has_no_solution,
                  "");
  TraceFunctionResultEnd();
  return hash_value==DirSucc ? has_solution : has_no_solution;
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

/* Determine and write keys leading to a double-mate
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_dmate_solve(slice_index leaf)
{
  boolean solution_found = false;
  Side const starter = slices[leaf].starter;

  if (!immobile(starter))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf)==goal_reached)
      {
        solution_found = true;
        write_final_attack(goal_doublemate,attack_key);
      }

      repcoup();
    }

    finply();
  }

  return solution_found;
}

/* Determine and write keys leading to counter-mate
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_cmate_solve(slice_index leaf)
{
  boolean solution_found = false;
  Side const starter = slices[leaf].starter;
  Side const non_starter = advers(starter);

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
        solution_found = true;
        write_final_attack(goal_countermate,attack_key);
      }
      repcoup();
    }

    finply();
  }

  return solution_found;
}

/* Determine and write keys leading to "regular goals"
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_regulargoals_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].starter;
  boolean solution_found = false;

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
      solution_found = true;
      write_final_attack(slices[leaf].u.leaf.goal,attack_key);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",solution_found);
  TraceFunctionResultEnd();
  return solution_found;
}

/* Determine and write keys
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
boolean leaf_d_solve(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  /* Only check for DirNoSucc - we also have to write the solution if
   * we already know that there is one!
   */
  if (inhash(leaf,DirNoSucc,1))
  {
    assert(!inhash(leaf,DirSucc,0));
    result = false;
  }
  else
  {
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

    if (result)
      addtohash(leaf,DirSucc,0);
    else
      addtohash(leaf,DirNoSucc,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write the key just played
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_d_root_write_key(slice_index leaf, attack_type type)
{
  write_final_attack(slices[leaf].u.leaf.goal,type);
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return whether starter has won
 */
has_starter_won_result_type leaf_d_has_starter_won(slice_index leaf)
{
  has_starter_won_result_type result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  switch (leaf_is_goal_reached(slices[leaf].starter,leaf))
  {
    case goal_not_reached:
      result = starter_has_not_won;
      break;

    case goal_not_reached_selfcheck:
      result = starter_has_not_won_selfcheck;
      break;

    case goal_reached:
      result = starter_has_won;
      break;

    default:
      assert(0);
      result = starter_has_not_won;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_starter_won_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param leaf slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_d_has_starter_reached_goal(slice_index leaf)
{
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  result = leaf_is_goal_reached(slices[leaf].starter,leaf)==goal_reached;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defender wins after a move by the attacker
 * @param leaf identifies leaf
 * @return true iff the defender wins
 */
boolean leaf_d_does_defender_win(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  result = leaf_d_has_solution(leaf)==has_no_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find and write post key play
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_d_solve_postkey(slice_index leaf)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find and write continuations and append them to the top table
 * @param leaf slice index
 */
void leaf_d_solve_continuations(slice_index leaf)
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

/* Write a priori unsolvability (if any) of a leaf (e.g. forced reflex
 * mates)
 * @param leaf leaf's slice index
 */
void leaf_d_write_unsolvability(slice_index leaf)
{
  output_start_continuation_level();
  leaf_d_solve_continuations(leaf);
  output_end_continuation_level();
  write_end_of_solution();
}

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_d_detect_starter(slice_index leaf,
                                             boolean same_side_as_root)
{
  who_decides_on_starter const result = leaf_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  if (slices[leaf].starter==no_side)
    slices[leaf].starter = White;

  TraceValue("%u\n",slices[leaf].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
