#include "pyslice.h"
#include "pydata.h"
#include "trace.h"
#include "pyleaf.h"
#include "pyleafd.h"
#include "pyleafs.h"
#include "pyleaff.h"
#include "pyleafh.h"
#include "pybrad.h"
#include "pybradd.h"
#include "pybrah.h"
#include "pybraser.h"
#include "pybrafrk.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pynot.h"
#include "pymovein.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pyselfcg.h"
#include "pypipe.h"

#include <assert.h>
#include <stdlib.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * Tests do not rely on the current position being hash-encoded.
 * @param si slice index
 * @return true iff starter must resign
 */
boolean slice_must_starter_resign(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_must_starter_resign(si);
      break;

    case STLeafHelp:
      result = leaf_h_must_starter_resign(si);
      break;

    case STLeafSelf:
      result = leaf_s_must_starter_resign(si);
      break;

    case STLeafForced:
      result = leaf_forced_must_starter_resign(si);
      break;

    case STReciprocal:
      result = reci_must_starter_resign(si);
      break;
      
    case STQuodlibet:
      result = quodlibet_must_starter_resign(si);
      break;
      
    case STNot:
      result = false;
      break;

    case STBranchDirect:
      result = branch_d_must_starter_resign(si);
      break;

    case STBranchDirectDefender:
      result = branch_d_defender_must_starter_resign(si);
      break;

    case STHelpRoot:
    case STHelpAdapter:
      result = help_adapter_must_starter_resign(si);
      break;

    case STBranchFork:
      result = branch_fork_must_starter_resign(si);
      break;

    case STSeriesRoot:
    case STSeriesAdapter:
    case STMoveInverter:
    case STSelfCheckGuard:
    case STGoalReachableGuard:
      result = pipe_must_starter_resign(si);
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @return true iff no chance is left
 */
boolean slice_must_starter_resign_hashed(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STReciprocal:
      result = reci_must_starter_resign_hashed(si);
      break;
      
    case STQuodlibet:
      result = quodlibet_must_starter_resign_hashed(si);
      break;
      
    case STNot:
      result = not_must_starter_resign_hashed(si);
      break;

    case STMoveInverter:
      result = move_inverter_must_starter_resign_hashed(si);
      break;

    case STBranchFork:
      result = branch_fork_must_starter_resign_hashed(si);
      break;

    case STHelpHashed:
      result = hashed_help_must_starter_resign_hashed(si);
      break;

    case STBranchDirect:
    case STHelpAdapter:
    case STBranchHelp:
    case STSeriesAdapter:
    case STBranchSeries:
    case STLeafDirect:
    case STLeafHelp:
    case STLeafSelf:
    case STLeafForced:
      break;

    default:
      assert(0);
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void slice_solve_continuations(table continuations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_solve_continuations(si);
      break;
    
    case STLeafSelf:
      leaf_s_solve_continuations(si);
      break;
    
    case STQuodlibet:
      quodlibet_solve_continuations(continuations,si);
      break;

    case STReciprocal:
      reci_solve_continuations(continuations,si);
      break;

    case STNot:
      not_solve_continuations(continuations,si);
      break;

    case STBranchDirect:
      branch_d_solve_continuations(continuations,si);
      break;

    case STHelpAdapter:
      help_adapter_solve_continuations(continuations,si);
      break;

    case STSeriesAdapter:
      series_adapter_solve_continuations(continuations,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index slice_root_make_setplay_slice(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpRoot:
      result = help_root_make_setplay_slice(si);
      break;

    case STBranchDirect:
      result = branch_d_root_make_setplay_slice(si);
      break;

    case STSeriesRoot:
      result = series_root_make_setplay_slice(si);
      break;

    case STLeafSelf:
      result = leaf_s_root_make_setplay_slice(si);
      break;

    case STReciprocal:
      result = reci_root_make_setplay_slice(si);
      break;

    case STQuodlibet:
      result = quodlibet_root_make_setplay_slice(si);
      break;

    case STNot:
      result = not_root_make_setplay_slice(si);
      break;

    case STMoveInverter:
      result = move_inverter_root_make_setplay_slice(si);
      break;

    default:
      result = no_slice;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void slice_root_write_key(slice_index si, attack_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_root_write_key(si,type);
      break;

    case STLeafSelf:
      leaf_s_root_write_key(si,type);
      break;

    case STQuodlibet:
      quodlibet_root_write_key(si,type);
      break;

    case STBranchDirect:
      branch_d_root_write_key(si,type);
      break;

    case STHelpAdapter:
      help_adapter_root_write_key(si,type);
      break;

    case STSeriesAdapter:
      series_adapter_root_write_key(si,type);
      break;

    case STReciprocal:
    {
      reci_root_write_key(si,type);
      break;
    }

    case STNot:
      /* STNot doesn't have got a key by definition */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve(slice_index si)
{
  boolean solution_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      solution_found = leaf_d_solve(si);
      break;

    case STLeafForced:
      solution_found = leaf_forced_solve(si);
      break;

    case STLeafHelp:
      solution_found = leaf_h_solve(si);
      break;

    case STLeafSelf:
      solution_found = leaf_s_solve(si);
      break;

    case STQuodlibet:
      solution_found = quodlibet_solve(si);
      break;

    case STBranchDirect:
      solution_found = branch_d_solve(si);
      break;

    case STHelpAdapter:
      solution_found = help_adapter_solve(si);
      break;

    case STSeriesAdapter:
      solution_found = series_adapter_solve(si);
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

    case STSelfCheckGuard:
      solution_found = selfcheck_guard_solve(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",solution_found);
  TraceFunctionResultEnd();
  return solution_found;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_root_solve(si);
      break;

    case STLeafHelp:
      result = leaf_h_root_solve(si);
      break;

    case STLeafSelf:
      result = leaf_s_root_solve(si);
      break;

    case STLeafForced:
      result = leaf_forced_root_solve(si);
      break;

    case STQuodlibet:
      result = quodlibet_root_solve(si);
      break;

    case STReciprocal:
      result = reci_root_solve(si);
      break;

    case STNot:
      result = not_root_solve(si);
      break;

    case STBranchDirect:
      result = branch_d_root_solve(si);
      break;

    case STBranchDirectDefender:
      result = branch_d_defender_root_solve(si);
      break;

    case STHelpRoot:
      result = help_root_solve(si);
      break;

    case STSeriesRoot:
      result = series_root_solve(si);
      break;

    case STMoveInverter:
      result = move_inverter_root_solve(si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_solve(si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_root_solve(si);
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void slice_root_solve_in_n(slice_index si, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpRoot:
      help_root_solve_in_n(si,n);
      break;

    case STSeriesRoot:
      series_root_solve_in_n(si,n);
      break;

    case STQuodlibet:
      quodlibet_root_solve_in_n(si,n);
      break;

    case STMoveInverter:
      move_inverter_root_solve_in_n(si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean slice_has_solution(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

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
      result = branch_d_has_solution(si);
      break;

    case STHelpRoot:
      result = help_root_has_solution(si);
      break;

    case STHelpAdapter:
      result = help_adapter_has_solution(si);
      break;

    case STSeriesAdapter:
      result = series_adapter_has_solution(si);
      break;

    case STSelfCheckGuard:
      result = pipe_has_solution(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find and write post key play
 * @param si slice index
 */
void slice_solve_postkey(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_solve_postkey(si);
      break;

    case STLeafSelf:
      leaf_s_solve_postkey(si);
      break;

    case STLeafHelp:
      leaf_h_solve_postkey(si);
      break;

    case STQuodlibet:
      quodlibet_solve_postkey(si);
      break;

    case STHelpAdapter:
      help_adapter_solve_postkey(si);
      break;

    case STSeriesAdapter:
      series_adapter_solve_postkey(si);
      break;

    case STBranchFork:
      branch_fork_solve_postkey(si);
      break;

    case STReciprocal:
      reci_solve_postkey(si);
      break;

    case STNot:
      /* STNot doesn't have postkey play by definition */
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
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
      result = branch_d_defender_has_non_starter_solved(si);
      break;

    case STHelpAdapter:
      result = help_adapter_has_non_starter_solved(si);
      break;

    case STSeriesAdapter:
      result = series_adapter_has_non_starter_solved(si);
      break;

    case STQuodlibet:
      result = quodlibet_has_non_starter_solved(si);
      break;

    case STReciprocal:
      result = reci_has_non_starter_solved(si);
      break;

    case STNot:
      result = not_has_non_starter_solved(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
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
      result = branch_d_has_starter_apriori_lost(si);
      break;

    case STHelpAdapter:
      result = help_adapter_has_starter_apriori_lost(si);
      break;

    case STSeriesAdapter:
      result = series_adapter_has_starter_apriori_lost(si);
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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
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
      result = branch_d_has_starter_won(si);
      break;
 
    case STHelpAdapter:
      result = help_adapter_has_starter_won(si);
      break;

    case STSeriesAdapter:
      result = series_adapter_has_starter_won(si);
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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
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
      result = branch_d_has_starter_reached_goal(si);
      break;

    case STHelpAdapter:
      result = help_adapter_has_starter_reached_goal(si);
      break;

    case STSeriesAdapter:
      result = series_adapter_has_starter_reached_goal(si);
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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafForced:
    case STLeafHelp:
      result = leaf_is_goal_reached(just_moved,si);
      break;

    case STBranchDirect:
      result = branch_d_is_goal_reached(just_moved,si);
      break;

    case STBranchDirectDefender:
      result = branch_d_defender_is_goal_reached(just_moved,si);
      break;

    case STBranchHelp:
    case STHelpHashed:
    case STSelfCheckGuard:
      result = branch_h_is_goal_reached(just_moved,si);
      break;

    case STHelpRoot:
    case STHelpAdapter:
      result = help_adapter_is_goal_reached(just_moved,si);
      break;

    case STSeriesRoot:
    case STSeriesAdapter:
      result = series_adapter_is_goal_reached(just_moved,si);
      break;

    case STBranchFork:
      result = branch_fork_is_goal_reached(just_moved,si);
      break;

    default:
      /* TODO */
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slices[si].type)
  {
    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      leaf_write_unsolvability(si);
      break;

    case STBranchDirect:
      branch_d_write_unsolvability(si);
      break;

    case STHelpAdapter:
      help_adapter_write_unsolvability(si);
      break;

    case STSeriesAdapter:
      series_adapter_write_unsolvability(si);
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
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter slice_detect_starter(slice_index si,
                                            boolean same_side_as_root)
{
  who_decides_on_starter result = dont_know_who_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",same_side_as_root);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_detect_starter(si,same_side_as_root);
      break;

    case STLeafSelf:
      result = leaf_s_detect_starter(si,same_side_as_root);
      break;

    case STLeafForced:
      result = leaf_forced_detect_starter(si,same_side_as_root);
      break;

    case STLeafHelp:
      result = leaf_h_detect_starter(si,same_side_as_root);
      break;

    case STBranchDirect:
      result = branch_d_detect_starter(si,same_side_as_root);
      break;

    case STBranchDirectDefender:
      result = branch_d_defender_detect_starter(si,same_side_as_root);
      break;

    case STHelpRoot:
    case STBranchHelp:
      result = branch_h_detect_starter(si,same_side_as_root);
      break;

    case STSeriesRoot:
    case STBranchSeries:
      result = branch_ser_detect_starter(si,same_side_as_root);
      break;

    case STBranchFork:
      result = branch_fork_detect_starter(si,same_side_as_root);
      break;

    case STReciprocal:
      result = reci_detect_starter(si,same_side_as_root);
      break;

    case STQuodlibet:
      result = quodlibet_detect_starter(si,same_side_as_root);
      break;

    case STMoveInverter:
      result = move_inverter_detect_starter(si,same_side_as_root);
      break;

    case STNot:
    case STHelpAdapter:
    case STReflexGuard:
    case STSeriesAdapter:
      result = pipe_detect_starter(si,same_side_as_root);
      break;
      
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write that the non-starter has solved (i.e. in a self stipulation)
 * @param si slice index
 */
void slice_write_non_starter_has_solved(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slices[si].type)
  {
    case STLeafSelf:
      leaf_s_write_non_starter_has_solved(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
