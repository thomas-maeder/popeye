#include "pyleafs.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyint.h"
#include "pyoutput.h"
#include "pyleaf.h"
#include "pyhash.h"

#include <assert.h>
#include <stdlib.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf leaf's slice index
 * @return true iff starter must resign
 */
boolean leaf_s_must_starter_resign(slice_index leaf)
{
  boolean result = false;
  Side const defender = advers(slices[leaf].u.leaf.starter);

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(defender);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether there is a solution in a self leaf.
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
boolean leaf_s_has_solution(slice_index leaf)
{
  boolean result = false;
  HashBuffer hb;
  Side const attacker = slices[leaf].u.leaf.starter;
  Side const defender = advers(attacker);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  (*encode)(&hb);
  /* It is more likely that a position has no solution. */
  /*    Therefore let's check for "no solution" first. TLi */
  if (inhash(leaf,DirNoSucc,1,&hb))
    assert(!inhash(leaf,DirSucc,0,&hb));
  else if (inhash(leaf,DirSucc,0,&hb))
    result = true;
  else
  {
    genmove(attacker);

    while (!result
           && encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove()
          && !echecc(nbply,attacker)
          && !(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
          && leaf_is_end_in_1_forced(leaf))
      {
        result = true;
        coupfort();
      }

      repcoup();

      if (maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    finply();

    if (result)
      addtohash(leaf,DirSucc,0,&hb);
    else
      addtohash(leaf,DirNoSucc,1,&hb);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a leaf slice.has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_s_has_non_starter_solved(slice_index leaf)
{
  Side const defender = advers(slices[leaf].u.leaf.starter);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceValue("%u\n",defender);

  result = leaf_is_goal_reached(defender,leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and find final moves of a self leaf
 * @param side_at_move side to perform the final move
 * @param leaf slice index
 * @return true iff >= 1 solution was found
 */
static boolean leaf_s_solve_final_move(slice_index leaf)
{
  boolean final_move_found = false;
  Side const side_at_move = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  TraceValue("%u\n",side_at_move);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,side_at_move);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && leaf_is_goal_reached(side_at_move,leaf))
    {
      final_move_found = true;
      write_final_defense(slices[leaf].u.leaf.goal);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",final_move_found);
  return final_move_found;
}

/* Determine and write solutions in a self stipulation in 1 move
 * @param leaf slice index of the leaf slice
 * @return true iff >=1 key was found and written
 */
boolean leaf_s_solve(slice_index leaf)
{
  boolean found_solution = false;
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
        && leaf_is_end_in_1_forced(leaf))
    {
      found_solution = true;

      write_attack(no_goal,attack_key);
      output_start_postkey_level();
      leaf_s_solve_final_move(leaf);
      output_end_postkey_level();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Write the key and solve the remainder of a leaf in direct play
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_s_root_write_key_solve_postkey(slice_index leaf, attack_type type)
{
  assert(slices[leaf].u.leaf.starter!=no_side);

  write_attack(no_goal,type);
  output_start_leaf_variation_level();
  if (OptFlag[solvariantes])
    leaf_s_solve_final_move(leaf);
  output_end_leaf_variation_level();
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_s_has_starter_apriori_lost(slice_index leaf)
{
  boolean result = false;
  Side const defender = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  result = OptFlag[keepmating] && !is_a_mating_piece_left(defender);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_s_has_starter_won(slice_index leaf)
{
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  result = leaf_is_end_in_1_forced(leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param leaf slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_s_has_starter_reached_goal(slice_index leaf)
{
  boolean const result = false;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write set play of a self/reflex stipulation.
 * @param leaf slice index of the leaf slice
 */
boolean leaf_s_root_solve_setplay(slice_index leaf)
{
  boolean result = false;
  Side const defender = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  TraceValue("%u\n",defender);

  active_slice[nbply+1] = leaf;
  generate_move_reaching_goal(leaf,defender);

  while(encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && leaf_is_goal_reached(defender,leaf))
    {
      result = true;
      write_final_defense(slices[leaf].u.leaf.goal);
      if (OptFlag[maxsols]) 
        solutions++;
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);
    }

    repcoup();

    if ((OptFlag[maxsols] && solutions>=maxsolutions)
        || maxtime_status==MAXTIME_TIMEOUT)
      break;
  }

  finply();

  return result;
}

/* Find and write set play provided every set move leads to end
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean leaf_s_root_solve_complete_set(slice_index leaf)
{
  boolean result = false;
  Side const defender = advers(slices[leaf].u.leaf.starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  assert(slices[leaf].u.leaf.starter!=no_side);

  if (!(OptFlag[keepmating] && !is_a_mating_piece_left(defender))
      && leaf_is_end_in_1_forced(leaf))
    result = leaf_s_root_solve_setplay(leaf);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write variations (i.e. nothing resp. defender's final
 * moves). 
 * @param leaf slice index
 */
void leaf_s_solve_variations(slice_index leaf)
{
  assert(slices[leaf].u.leaf.starter!=no_side);

  output_start_leaf_variation_level();
  leaf_s_solve_final_move(leaf);
  output_end_leaf_variation_level();
}

/* Find and write continuations (i.e. final move pairs)
 * @param solutions table where to append continuations found and written
 * @param leaf slice index
 */
void leaf_s_solve_continuations(int solutions, slice_index leaf)
{
  Side const attacker = slices[leaf].u.leaf.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove()
        && !echecc(nbply,attacker)
        && leaf_is_end_in_1_forced(leaf))
    {
      write_attack(no_goal,attack_regular);
      output_start_postkey_level();
      leaf_s_solve_final_move(leaf);
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
void leaf_s_detect_starter(slice_index leaf, boolean is_duplex)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParam("%u\n",is_duplex);

  if (slices[leaf].u.leaf.starter==no_side)
    slices[leaf].u.leaf.starter = is_duplex ? Black : White;

  TraceFunctionExit(__func__);
  TraceText("\n");
}
