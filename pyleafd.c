#include "pyleafd.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyoutput.h"
#include "pyleaf.h"
#include "pyhash.h"
#include "pyconst.h"
#include "pymsg.h"
#include "pyoutput.h"

#include <assert.h>
#include <stdlib.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf leaf's slice index
 * @return true iff starter must resign
 */
boolean leaf_d_must_starter_resign(slice_index leaf)
{
  boolean result = false;
  Side const attacker = slices[leaf].u.leaf.starter;

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(attacker);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether there is a solution in a direct leaf.
 * @param leaf slice index of leaf slice
 * @return true iff attacker can end in 1 move
 */
boolean leaf_d_has_solution(slice_index leaf)
{
  hashwhat result = nr_hashwhat;
  Side const attacker = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

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
      result = DirNoSucc;
    }
    else if (inhash(leaf,DirSucc,0))
      result = DirSucc;
  }

  if (result==nr_hashwhat)
  {
    if (OptFlag[keepmating] && !is_a_mating_piece_left(attacker))
    {
      TraceText("!is_a_mating_piece_left\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%u\n",false);
      return false;
    }

    if (slices[leaf].u.leaf.goal==goal_doublemate
        && immobile(attacker))
    {
      TraceText("attacker is immobile\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%u\n",false);
      return false;
    }

    generate_move_reaching_goal(leaf,attacker);

    while (encore() && result!=DirSucc)
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && leaf_is_goal_reached(attacker,leaf))
      {
        result = DirSucc;
        coupfort();
      }

      repcoup();

      if (maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    finply();

    if (!FlagMoveOrientatedStip)
    {
      if (result==DirSucc)
        addtohash(leaf,DirSucc,0);
      else
        addtohash(leaf,DirNoSucc,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result==DirSucc);
  return result==DirSucc;
}

/* Determine whether a leaf slice.has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_d_has_non_starter_solved(slice_index leaf)
{
  boolean const result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write keys leading to a double-mate
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_root_dmate_solve(slice_index leaf)
{
  Side const starter = slices[leaf].u.leaf.starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  if (!immobile(starter))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    solutions = 0;

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf))
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
        TraceValue("%u",maxsolutions);
        TraceValue("%u",solutions);
        TraceText("aborting\n");

        /* signal maximal number of solutions reached to outer world */
        FlagMaxSolsReached = true;

        break;
      }
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return true;
}

/* Determine and write keys leading to counter-mate
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_root_cmate_solve(slice_index leaf)
{
  Side const starter = slices[leaf].u.leaf.starter;
  Side const non_starter = advers(starter);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  /* TODO can this be generalised to non-mate goals? */
  if (goal_checker_mate(non_starter))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    solutions = 0;

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf))
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
        TraceValue("%u",maxsolutions);
        TraceValue("%u",solutions);
        TraceText("aborting\n");

        /* signal maximal number of solutions reached to outer world */
        FlagMaxSolsReached = true;

        break;
      }
    }

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write keys leading to "regular goals"
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_root_regulargoals_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,attacker);

  solutions = 0;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && leaf_is_goal_reached(attacker,leaf))
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
      TraceValue("%u",maxsolutions);
      TraceValue("%u",solutions);
      TraceText("aborting\n");

      /* signal maximal number of solutions reached to outer world */
      FlagMaxSolsReached = true;

      break;
    }
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write keys at root level
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
boolean leaf_d_root_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  init_output(leaf);

  output_start_continuation_level();

  if (echecc(nbply,advers(attacker)))
  {
    ErrorMsg(KingCapture);
    result = false;
  }
  else
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
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write keys leading to a double-mate
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
static boolean leaf_d_dmate_solve(slice_index leaf)
{
  boolean solution_found = false;
  Side const starter = slices[leaf].u.leaf.starter;

  if (!immobile(starter))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf))
      {
        solution_found = true;
        output_start_continuation_level();
        write_final_attack(goal_doublemate,attack_key);
        output_start_leaf_variation_level();
        output_end_leaf_variation_level();
        output_end_continuation_level();
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
  Side const starter = slices[leaf].u.leaf.starter;
  Side const non_starter = advers(starter);

  /* TODO can this be generalised to non-mate goals? */
  if (goal_checker_mate(non_starter))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,starter);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(starter,leaf))
      {
        solution_found = true;
        output_start_continuation_level();
        write_final_attack(goal_countermate,attack_key);
        output_start_leaf_variation_level();
        output_end_leaf_variation_level();
        output_end_continuation_level();
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
  Side const attacker = slices[leaf].u.leaf.starter;
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && leaf_is_goal_reached(attacker,leaf))
    {
      solution_found = true;
      output_start_continuation_level();
      write_final_attack(slices[leaf].u.leaf.goal,attack_key);
      output_start_leaf_variation_level();
      output_end_leaf_variation_level();
      output_end_continuation_level();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",solution_found);
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
  TraceFunctionParam("%u\n",leaf);

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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_d_has_starter_apriori_lost(slice_index leaf)
{
  boolean const result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_d_has_starter_won(slice_index leaf)
{
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = leaf_is_goal_reached(slices[leaf].u.leaf.starter,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  TraceFunctionParam("%u\n",leaf);

  result = leaf_is_goal_reached(slices[leaf].u.leaf.starter,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write post key play
 * @param leaf slice index
 */
void leaf_d_solve_postkey(slice_index leaf)
{
  output_start_leaf_variation_level();
  output_end_leaf_variation_level();
}

/* Find and write continuations and append them to the top table
 * @param leaf slice index
 */
void leaf_d_solve_continuations(slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,attacker);

  while (encore())
  {
    /* TODO optimise echecc() check into leaf_is_goal_reached? */
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,attacker)
        && leaf_is_goal_reached(attacker,leaf))
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
  TraceText("\n");
}

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_d_detect_starter(slice_index leaf,
                                             boolean same_side_as_root)
{
  who_decides_on_starter result = dont_know_who_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u\n",same_side_as_root);

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_proof:
    case goal_atob:
      slices[leaf].u.leaf.starter = same_side_as_root ? White : Black;
      result = leaf_decides_on_starter;
      break;

    default:
      /* normally White, but Black in reci-h -> let somebody impose
       * the starter unless we are at the root level */
      if (leaf==root_slice)
        slices[leaf].u.leaf.starter = same_side_as_root ? White : Black;
      break;
  }

  TraceValue("%u\n",slices[leaf].u.leaf.starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
