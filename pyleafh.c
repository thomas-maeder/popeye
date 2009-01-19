#include "pyleafh.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyint.h"
#include "pyleaf.h"
#include "pyoutput.h"
#include "pyhash.h"

#include <assert.h>
#include <stdlib.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf leaf's slice index
 * @return true iff starter must resign
 */
boolean leaf_h_must_starter_resign(slice_index leaf)
{
  boolean result = false;
  Side const final = advers(slices[leaf].u.leaf.starter);

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(final);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether there is a solution in a help leaf.
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
boolean leaf_h_has_solution(slice_index leaf)
{
  boolean result = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(side_at_move);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  genmove(side_at_move);

  if (side_at_move==White)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && (!isIntelligentModeActive || isGoalReachable())
        && !echecc(nbply,side_at_move)
        && !(OptFlag[keepmating] && !is_a_mating_piece_left(other_side)))
    {
      if (compression_counter==0)
      {
        (*encode)();
        if (!inhash(leaf,HelpNoSuccOdd,1))
        {
          if (leaf_is_end_in_1_possible(other_side,leaf))
          {
            result = true;
            coupfort();
          }
          else
            addtohash(leaf,HelpNoSuccOdd,1);
        }
      }
      else if (leaf_is_end_in_1_possible(other_side,leaf))
      {
        result = true;
        coupfort();
      }
    }

    repcoup();

    if (maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  if (side_at_move==White)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a leaf slice.has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_h_has_non_starter_solved(slice_index leaf)
{
  Side const final = advers(slices[leaf].u.leaf.starter);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceValue("%u\n",final);

  result = leaf_is_goal_reached(final,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}


#if !defined(DATABASE)
/* Determine and find final moves of a help leaf
 * @param side_at_move side to perform the final move
 * @param leaf slice index
 * @return true iff >= 1 solution was found
 */
static boolean leaf_h_solve_final_move(slice_index leaf)
{
  boolean final_move_found = false;
  Side const side_at_move = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  TraceValue("%u\n",side_at_move);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,side_at_move);

  if (side_at_move==White)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !(isIntelligentModeActive && !isGoalReachable())
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      final_move_found = true;
      write_final_help_move(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  if (side_at_move==White)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",final_move_found);
  return final_move_found;
}
#endif

/* Solve the final move for a countermate
 * @param leaf identifies leaf slice
 * @return true iff >=1 moves have been found and written
 */
static boolean leaf_h_cmate_solve_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(slices[leaf].u.leaf.starter);

  if (goal_checker_mate(side_at_move))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,other_side);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(other_side,leaf))
      {
        found_solution = true;
        write_final_help_move(goal_countermate);
      }
      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Determine and write the final move pair in help countermate.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_cmate_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,side_at_move);

  while (encore())
  {
    if (jouecoup(nbply,first_play)
        && !echecc(nbply,side_at_move))
    {
      if (compression_counter==0)
      {
        (*encode)();
        if (!inhash(leaf,HelpNoSuccOdd,1))
        {
          if (leaf_h_cmate_solve_final_move(leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1);
        }
      }
      else if (leaf_h_cmate_solve_final_move(leaf))
        found_solution = true;
    }

    repcoup();
  }

  finply();

  return found_solution;
}

/* Solve the final move for a doublemate
 * @param leaf identifies leaf slice
 * @return true iff >=1 moves have been found and written
 */
static boolean leaf_h_dmate_solve_final_move(slice_index leaf)
{
  boolean found_solution = false;
  Side const final_side = advers(slices[leaf].u.leaf.starter);

  if (!immobile(final_side))
  {
    active_slice[nbply+1] = leaf;
    generate_move_reaching_goal(leaf,final_side);

    while (encore())
    {
      if (jouecoup(nbply,first_play)
          && leaf_is_goal_reached(final_side,leaf))
      {
        found_solution = true;
        write_final_help_move(goal_doublemate);
      }

      repcoup();
    }

    finply();
  }

  return found_solution;
}

/* Determine and write the final move pair in help doublemate.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_dmate_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;

  active_slice[nbply+1] = leaf;
  genmove(side_at_move);

  while (encore())
  {
    if (jouecoup(nbply,first_play)
        && !echecc(nbply,side_at_move))
    {
      if (compression_counter==0)
      {
        (*encode)();
        if (!inhash(leaf,HelpNoSuccOdd,1))
        {
          if (leaf_h_dmate_solve_final_move(leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1);
        }
      }
      else if (leaf_h_dmate_solve_final_move(leaf))
        found_solution = true;
    }

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  return found_solution;
}

/* Determine and write the final move pair in help stipulation with
 * "regular" goal. 
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
static boolean leaf_h_regulargoals_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const side_at_move = slices[leaf].u.leaf.starter;
  Side const other_side = advers(side_at_move);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  genmove(side_at_move);

  if (side_at_move==Black)
    BlMovesLeft--;
  else
    WhMovesLeft--;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && (!isIntelligentModeActive || isGoalReachable())
        && !echecc(nbply,side_at_move)
        && (!OptFlag[keepmating] || is_a_mating_piece_left(other_side)))
    {
      if (compression_counter==0)
      {
        (*encode)();
        if (!inhash(leaf,HelpNoSuccOdd,1))
        {
          if (leaf_h_solve_final_move(leaf))
            found_solution = true;
          else
            addtohash(leaf,HelpNoSuccOdd,1);
        }
      }
      else if (leaf_h_solve_final_move(leaf))
        found_solution = true;
    }

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }
    
  if (side_at_move==Black)
    BlMovesLeft++;
  else
    WhMovesLeft++;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Determine and write the solution of a help leaf slice.
 * @param leaf slice index
 * @return true iff >=1 move pair was found
 */
boolean leaf_h_solve(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  switch (slices[leaf].u.leaf.goal)
  {
    case goal_countermate:
      result = leaf_h_cmate_solve(leaf);
      break;

    case goal_doublemate:
      result = leaf_h_dmate_solve(leaf);
      break;

    default:
      result = leaf_h_regulargoals_solve(leaf);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write the key and solve the remainder of a leaf in direct play
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_h_root_write_key_solve_postkey(slice_index leaf, attack_type type)
{
  assert(slices[leaf].u.leaf.starter!=no_side);

  write_attack(no_goal,type);
  output_start_leaf_variation_level();
  if (OptFlag[solvariantes])
    leaf_h_solve_final_move(leaf);
  output_end_leaf_variation_level();
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_h_has_starter_apriori_lost(slice_index leaf)
{
  boolean result = false;
  Side const final = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(final);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_h_has_starter_won(slice_index leaf)
{
  boolean result;
  Side const final = advers(slices[leaf].u.leaf.starter);
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  result = (!(OptFlag[keepmating] && !is_a_mating_piece_left(final))
            && leaf_is_end_in_1_possible(final,leaf));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param leaf slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_h_has_starter_reached_goal(slice_index leaf)
{
  boolean const result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write defender's set play
 * @param leaf slice index
 */
boolean leaf_h_root_solve_setplay(slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  result = leaf_h_solve_final_move(leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write set play provided every set move leads to end
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean leaf_h_root_solve_complete_set(slice_index leaf)
{
  boolean result = false;
  Side const final = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  if (!(OptFlag[keepmating] && !is_a_mating_piece_left(final)))
    result = leaf_h_root_solve_setplay(leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write variations (i.e. nothing resp. defender's final
 * moves). 
 * @param leaf slice index
 */
void leaf_h_solve_variations(slice_index leaf)
{
  assert(slices[leaf].u.leaf.starter!=no_side);

  output_start_leaf_variation_level();
  leaf_h_solve_final_move(leaf);
  output_end_leaf_variation_level();
}

/* Find and write continuations (i.e. final move pairs)
 * @param solutions table where to append continuations found and written
 * @param leaf slice index
 */
void leaf_h_solve_continuations(int solutions, slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;
  Side const defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && !(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
        && leaf_is_end_in_1_possible(defender,leaf))
    {
      write_attack(no_goal,attack_regular);
      output_start_postkey_level();
      leaf_h_solve_final_move(leaf);
      output_end_postkey_level();
      pushtabsol(solutions);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param is_duplex is this for duplex?
 */
void leaf_h_detect_starter(slice_index leaf, boolean is_duplex)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u\n",is_duplex);

  if (slices[leaf].u.leaf.starter==no_side)
    slices[leaf].u.leaf.starter = is_duplex ? White : Black;

  TraceFunctionExit(__func__);
  TraceText("\n");
}
