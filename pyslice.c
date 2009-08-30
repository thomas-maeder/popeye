#include "pyslice.h"
#include "pydata.h"
#include "trace.h"
#include "pyleaf.h"
#include "pyleafd.h"
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
#include "pydirctg.h"
#include "pyselfgd.h"
#include "pyselfcg.h"
#include "pykeepmt.h"
#include "pypipe.h"

#include <assert.h>
#include <stdlib.h>


#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                             \
  ENUMERATOR(defender_self_check),              \
    ENUMERATOR(has_solution),                   \
    ENUMERATOR(has_no_solution)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


#define ENUMERATION_TYPENAME has_starter_won_result_type
#define ENUMERATORS                             \
  ENUMERATOR(starter_has_not_won),              \
    ENUMERATOR(starter_has_not_won_selfcheck),  \
    ENUMERATOR(starter_has_won)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


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

    case STSelfCheckGuard:
      selfcheck_guard_solve_continuations(continuations,si);
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

    case STDirectRoot:
      result = direct_root_make_setplay_slice(si);
      break;

    case STSeriesRoot:
      result = series_root_make_setplay_slice(si);
      break;

    case STMoveInverter:
      result = move_inverter_root_make_setplay_slice(si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_make_setplay_slice(si);
      break;

    case STSelfCheckGuard:
      result = slice_root_make_setplay_slice(slices[si].u.pipe.next);
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

    case STLeafHelp:
      break;

    case STLeafForced:
      leaf_forced_root_write_key(si,type);
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
      reci_root_write_key(si,type);
      break;

    case STNot:
      /* STNot doesn't have got a key by definition */
      break;

    case STDirectAttack:
      direct_attack_root_write_key(si,type);
      break;

    case STSelfAttack:
      self_attack_root_write_key(si,type);
      break;

    case STSelfCheckGuard:
      selfcheck_guard_root_write_key(si,type);
      break;

    case STReflexGuard:
      reflex_guard_root_write_key(si,type);
      break;

    case STKeepMatingGuard:
      keepmating_guard_root_write_key(si,type);
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

    case STDirectDefense:
      solution_found = direct_defense_solve(si);
      break;

    case STSelfDefense:
      solution_found = self_defense_solve(si);
      break;

    case STReflexGuard:
      solution_found = reflex_guard_solve(si);
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

    case STDirectRoot:
      result = direct_root_solve(si);
      break;

    case STDirectDefenderRoot:
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

    case STDirectAttack:
      result = direct_guard_root_solve(si);
      break;

    case STSelfAttack:
      result = self_guard_root_solve(si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_solve(si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_root_solve(si);
      break;

    case STBranchFork:
      result = branch_fork_root_solve(si);
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

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean slice_are_threats_refuted(table threats, slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_are_threats_refuted(threats,si);
      break;

    case STReciprocal:
      result = reci_are_threats_refuted(threats,si);
      break;

    case STQuodlibet:
      result = quodlibet_are_threats_refuted(threats,si);
      break;

    case STHelpAdapter:
    case STHelpHashed:
      result = help_adapter_are_threats_refuted(threats,si);
      break;

    case STBranchHelp:
      result = branch_h_are_threats_refuted(threats,si);
      break;

    case STSeriesAdapter:
    case STSeriesHashed:
      result = series_adapter_are_threats_refuted(threats,si);
      break;

    case STBranchSeries:
      result = branch_ser_are_threats_refuted(threats,si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_are_threats_refuted(threats,si);
      break;

    case STNot:
      result = true;
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

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_has_solution(slice_index si)
{
  has_solution_type result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_has_solution(si);
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

    case STDirectRoot:
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

    case STBranchFork:
      result = branch_fork_has_solution(si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_has_solution(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Find and write post key play
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve_postkey(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_solve_postkey(si);
      break;

    case STLeafForced:
      result = leaf_forced_solve_postkey(si);
      break;

    case STLeafHelp:
      result = leaf_h_solve_postkey(si);
      break;

    case STQuodlibet:
      result = quodlibet_solve_postkey(si);
      break;

    case STHelpAdapter:
      result = help_adapter_solve_postkey(si);
      break;

    case STSeriesAdapter:
      result = series_adapter_solve_postkey(si);
      break;

    case STReciprocal:
      result = reci_solve_postkey(si);
      break;

    case STNot:
      /* STNot doesn't have postkey play by definition */
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

    case STLeafForced:
      result = leaf_forced_has_non_starter_solved(si);
      break;

    case STLeafHelp:
      result = leaf_h_has_non_starter_solved(si);
      break;

    case STBranchDirect:
      result = branch_d_has_non_starter_solved(si);
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

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
has_starter_won_result_type slice_has_starter_won(slice_index si)
{
  has_starter_won_result_type result = starter_has_not_won;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      result = leaf_d_has_starter_won(si);
      break;

    case STLeafForced:
      result = leaf_forced_has_starter_won(si);
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
  TraceEnumerator(has_starter_won_result_type,result,"");
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

    case STLeafForced:
      leaf_forced_has_starter_reached_goal(si);
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
    case STLeafForced:
    case STLeafHelp:
      result = leaf_is_goal_reached(just_moved,si)==goal_reached;
      break;

    case STDirectRoot:
    case STBranchDirect:
      result = branch_d_is_goal_reached(just_moved,si);
      break;

    case STDirectDefenderRoot:
    case STBranchDirectDefender:
      result = branch_d_defender_is_goal_reached(just_moved,si);
      break;

    case STHelpRoot:
    case STHelpAdapter:
      result = help_adapter_is_goal_reached(just_moved,si);
      break;

    case STSeriesRoot:
    case STSeriesAdapter:
      result = series_adapter_is_goal_reached(just_moved,si);
      break;

    case STBranchHelp:
    case STHelpHashed:
    case STSelfCheckGuard:
      result = branch_h_is_goal_reached(just_moved,si);
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
 * @param si slice index
 */
void slice_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_write_unsolvability(si);
      break;

    case STLeafHelp:
      leaf_h_write_unsolvability(si);
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

    case STLeafForced:
      result = leaf_forced_detect_starter(si,same_side_as_root);
      break;

    case STLeafHelp:
      result = leaf_h_detect_starter(si,same_side_as_root);
      break;

    case STDirectRoot:
    case STBranchDirect:
      result = branch_d_detect_starter(si,same_side_as_root);
      break;

    case STDirectDefenderRoot:
      result = branch_d_defender_root_detect_starter(si,same_side_as_root);
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
    case STDirectAttack:
    case STSelfAttack:
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

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafForced:
      leaf_forced_write_non_starter_has_solved(si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the defender wins after a move by the attacker
 * @param si slice index
 * @return true iff defender wins
 */
boolean slice_does_defender_win(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafForced:
      result = leaf_forced_does_defender_win(si);
      break;

    case STLeafDirect:
      result = leaf_d_does_defender_win(si);
      break;

    case STLeafHelp:
      result = leaf_h_does_defender_win(si);
      break;

    case STQuodlibet:
      result = quodlibet_does_defender_win(si);
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
