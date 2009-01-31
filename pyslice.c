#include "pyslice.h"
#include "pydata.h"
#include "trace.h"
#include "pyleaf.h"
#include "pyleafd.h"
#include "pyleafs.h"
#include "pyleafh.h"
#include "pybrad.h"
#include "pybrah.h"
#include "pybraser.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pynot.h"
#include "pybranch.h"
#include "pymovein.h"

#include <assert.h>
#include <stdlib.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean slice_must_starter_resign(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_must_starter_resign(si);
      break;

    case STLeafSelf:
      result = leaf_s_must_starter_resign(si);
      break;

    case STLeafHelp:
      result = leaf_h_must_starter_resign(si);
      break;

    case STReciprocal:
      result = reci_must_starter_resign(si);
      break;
      
    case STQuodlibet:
      result = quodlibet_must_starter_resign(si);
      break;
      
    case STNot:
      result = not_must_starter_resign(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_must_starter_resign(si);
      break;

    case STMoveInverter:
      result = move_inverter_must_starter_resign(si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write continuations of a slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void slice_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_solve_continuations(table,si);
      break;
    
    case STLeafSelf:
      leaf_s_solve_continuations(table,si);
      break;
    
    case STQuodlibet:
      quodlibet_solve_continuations(table,si);
      break;

    case STReciprocal:
      reci_solve_continuations(table,si);
      break;

    case STNot:
      not_solve_continuations(table,si);
      break;

    case STBranchDirect:
      branch_d_solve_continuations_in_n(table,si,slices[si].u.branch.length);
      break;

    case STBranchHelp:
      branch_h_solve_continuations(table,si);
      break;

    case STBranchSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean slice_root_solve_setplay(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_root_solve_setplay(si);
      break;

    case STLeafSelf:
      result = leaf_s_root_solve_setplay(si);
      break;

    case STQuodlibet:
      result = quodlibet_root_solve_setplay(si);
      break;

    case STBranchDirect:
      result = branch_d_root_solve_setplay(si);
      break;

    case STBranchHelp:
      result = branch_h_root_solve_setplay(si);
      break;

    case STBranchSeries:
      /* TODO implement branch_ser_root_solve_setplay() */
      break;

    case STReciprocal:
      result = reci_root_solve_setplay(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean slice_root_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_root_solve_complete_set(si);
      break;

    case STLeafSelf:
      result = leaf_s_root_solve_complete_set(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_root_solve_complete_set(si);
      break;

    case STQuodlibet:
      result = quodlibet_root_solve_complete_set(si);
      break;

    case STReciprocal:
      /* not really meaningful */
      break;
      
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write the key just played, then continue solving in the slice
 * to find and write the post key play (threats, variations)
 * @param si slice index
 * @param type type of attack
 */
void slice_root_write_key_solve_postkey(slice_index si, attack_type type)
{
  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_root_write_key_solve_postkey(si,type);
      break;

    case STLeafSelf:
      leaf_s_root_write_key_solve_postkey(si,type);
      break;

    case STQuodlibet:
      quodlibet_root_write_key_solve_postkey(si,type);
      break;

    case STBranchDirect:
    {
      int const refutations = alloctab();
      branch_d_root_write_key_solve_postkey(refutations,si,type);
      freetab();
      break;
    }

    case STBranchHelp:
      /* TODO */
      break;

    case STBranchSeries:
      /* TODO */
      break;

    case STReciprocal:
    {
      reci_root_write_key_solve_postkey(si,type);
      break;
    }

    default:
      assert(0);
      break;
  }
}

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve(slice_index si)
{
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafDirect:
      solution_found = leaf_d_solve(si);
      break;

    case STLeafSelf:
      solution_found = leaf_s_solve(si);
      break;

    case STLeafHelp:
      solution_found = leaf_h_solve(si);
      break;

    case STQuodlibet:
      solution_found = quodlibet_solve(si);
      break;

    case STBranchDirect:
      /* TODO */
      break;

    case STBranchHelp:
      solution_found = branch_h_solve(si);
      break;

    case STBranchSeries:
      solution_found = branch_ser_solve(si);
      break;

    case STReciprocal:
      solution_found = reci_solve(si);
      break;

    case STNot:
      solution_found = not_solve(si);
      break;

    case STMoveInverter:
      solution_found = move_inverter_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",solution_found);
  return solution_found;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
void slice_root_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_solve(si);
      break;

    case STLeafSelf:
      leaf_s_solve(si);
      break;

    case STLeafHelp:
      leaf_h_solve(si);
      break;

    case STQuodlibet:
      quodlibet_root_solve(si);
      break;

    case STReciprocal:
      reci_root_solve(si);
      break;

    case STNot:
      not_root_solve(si);
      break;

    case STBranchDirect:
      branch_d_root_solve(si);
      break;

    case STBranchHelp:
      branch_h_root_solve(si);
      break;

    case STBranchSeries:
      branch_ser_root_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void slice_root_solve_in_n(slice_index si, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",n);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STBranchHelp:
      branch_h_root_solve_in_n(si,n);
      break;

    case STBranchSeries:
      branch_ser_root_solve_in_n(si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean slice_has_solution(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_has_solution(si);
      break;

    case STLeafSelf:
      result = leaf_s_has_solution(si);
      break;

    case STLeafHelp:
      result = leaf_h_has_solution(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_solution(si);
      break;

    case STReciprocal:
      result = reci_has_solution(si);
      break;

    case STNot:
      result = not_has_solution(si);
      break;

    case STBranchDirect:
      result = branch_d_has_solution_in_n(si,slices[si].u.branch.length);
      break;

    case STBranchHelp:
      result = branch_h_has_solution(si);
      break;

    case STBranchSeries:
      /* TODO */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write variations
 * @param si slice index
 */
void slice_solve_variations(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_solve_variations(si);
      break;

    case STLeafSelf:
      leaf_s_solve_variations(si);
      break;

    case STLeafHelp:
      leaf_h_solve_variations(si);
      break;

    case STQuodlibet:
      quodlibet_solve_variations(si);
      break;

    case STBranchDirect:
      branch_d_solve_variations(si);
      break;

    case STBranchHelp:
      /* TODO */
      break;

    case STBranchSeries:
      /* TODO */
      break;

    case STReciprocal:
      reci_solve_variations(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean slice_has_non_starter_solved(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_has_non_starter_solved(si);
      break;

    case STLeafSelf:
      result = leaf_s_has_non_starter_solved(si);
      break;

    case STLeafHelp:
      result = leaf_h_has_non_starter_solved(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_has_non_starter_solved(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_non_starter_solved(si);
      break;

    case STReciprocal:
      result = reci_has_non_starter_solved(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean slice_has_starter_apriori_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_has_starter_apriori_lost(si);
      break;

    case STLeafSelf:
      result = leaf_s_has_starter_apriori_lost(si);
      break;

    case STLeafHelp:
      result = leaf_h_has_starter_apriori_lost(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_has_starter_apriori_lost(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_starter_apriori_lost(si);
      break;

    case STReciprocal:
      result = reci_has_starter_apriori_lost(si);
      break;

    case STNot:
      result = not_has_starter_apriori_lost(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean slice_has_starter_won(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_has_starter_won(si);
      break;

    case STLeafSelf:
      result = leaf_s_has_starter_won(si);
      break;

    case STLeafHelp:
      result = leaf_h_has_starter_won(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_has_starter_won(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_starter_won(si);
      break;

    case STReciprocal:
      result = reci_has_starter_won(si);
      break;

    case STNot:
      result = not_has_starter_won(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean slice_has_starter_reached_goal(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_has_starter_reached_goal(si);
      break;

    case STLeafSelf:
      result = leaf_s_has_starter_reached_goal(si);
      break;

    case STLeafHelp:
      result = leaf_h_has_starter_reached_goal(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_has_starter_reached_goal(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_starter_reached_goal(si);
      break;

    case STReciprocal:
      result = reci_has_starter_reached_goal(si);
      break;

    case STNot:
      result = not_has_starter_reached_goal(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean slice_is_goal_reached(Side just_moved, slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      result = leaf_is_goal_reached(just_moved,si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_is_goal_reached(just_moved,si);
      break;

    default:
      /* TODO */
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice in direct play
 * (e.g. forced reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void slice_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      leaf_write_unsolvability(si);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      branch_write_unsolvability(si);
      break;

    case STQuodlibet:
      quodlibet_write_unsolvability(si);
      break;

    case STReciprocal:
      reci_write_unsolvability(si);
      break;

    case STNot:
      not_write_unsolvability(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter slice_detect_starter(slice_index si,
                                            boolean is_duplex,
                                            boolean same_side_as_root)
{
  who_decides_on_starter result = dont_know_who_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",same_side_as_root);

  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_detect_starter(si,is_duplex,same_side_as_root);
      break;

    case STLeafSelf:
      result = leaf_s_detect_starter(si,is_duplex,same_side_as_root);
      break;

    case STLeafHelp:
      result = leaf_h_detect_starter(si,is_duplex,same_side_as_root);
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = branch_detect_starter(si,is_duplex,same_side_as_root);
      break;

    case STReciprocal:
      result = reci_detect_starter(si,is_duplex,same_side_as_root);
      break;

    case STQuodlibet:
      result = quodlibet_detect_starter(si,is_duplex,same_side_as_root);
      break;

    case STNot:
      result = not_detect_starter(si,is_duplex,same_side_as_root);
      break;

    case STMoveInverter:
      result = move_inverter_detect_starter(si,is_duplex,same_side_as_root);
      break;
      
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void slice_impose_starter(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",side);

  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      leaf_impose_starter(si,side);
      break;

    case STBranchDirect:
      branch_d_impose_starter(si,side);
      break;

    case STBranchHelp:
      branch_h_impose_starter(si,side);
      break;

    case STBranchSeries:
      branch_ser_impose_starter(si,side);
      break;

    case STReciprocal:
      reci_impose_starter(si,side);
      break;

    case STQuodlibet:
      quodlibet_impose_starter(si,side);
      break;

    case STNot:
      not_impose_starter(si,side);
      break;

    case STMoveInverter:
      move_inverter_impose_starter(si,side);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Retrieve the starting side of a slice
 * @param si slice index
 * @return current starting side of slice si
 */
Side slice_get_starter(slice_index si)
{
  Side result = no_side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      result = slices[si].u.leaf.starter;
      break;

    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      result = slices[si].u.branch.starter;
      break;

    case STReciprocal:
      assert(slice_get_starter(slices[si].u.reciprocal.op1)
             ==slice_get_starter(slices[si].u.reciprocal.op2));
      result = slice_get_starter(slices[si].u.reciprocal.op1);
      break;

    case STQuodlibet:
      assert(slice_get_starter(slices[si].u.quodlibet.op1)
             ==slice_get_starter(slices[si].u.quodlibet.op2));
      result = slice_get_starter(slices[si].u.quodlibet.op1);
      break;

    case STNot:
      result = slice_get_starter(slices[si].u.not.op);
      break;

    case STMoveInverter:
    {
      slice_index const next = slices[si].u.move_inverter.next;
      Side const next_starter = slice_get_starter(next);
      if (next_starter!=no_side)
        result = advers(next_starter);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}
