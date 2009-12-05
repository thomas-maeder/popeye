#include "pyslice.h"
#include "pydata.h"
#include "trace.h"
#include "pyhelp.h"
#include "pyseries.h"
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


#define ENUMERATION_TYPENAME quantity_of_refutations_type
#define ENUMERATORS                             \
  ENUMERATOR(attacker_has_solved_next_slice),   \
    ENUMERATOR(found_no_refutation),            \
    ENUMERATOR(found_refutations),              \
    ENUMERATOR(attacker_has_reached_deadend)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void slice_solve_threats(table threats, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
      leaf_d_solve_threats(threats,si);
      break;
    
    case STQuodlibet:
      quodlibet_solve_threats(threats,si);
      break;

    case STReciprocal:
      reci_solve_threats(threats,si);
      break;

    case STNot:
      not_solve_threats(threats,si);
      break;

    case STDirectRoot:
    case STBranchDirect:
    case STDirectDefense:
    case STSelfDefense:
    case STSelfCheckGuard:
    case STReflexGuard:
    case STKeepMatingGuard:
    case STDegenerateTree:
    case STDirectHashed:
      direct_solve_threats(threats,si);
      break;

    case STBranchHelp:
    case STHelpHashed:
      help_solve_threats(threats,si);
      break;

    case STBranchSeries:
    case STSeriesHashed:
      series_solve_threats(threats,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find refutations after a move of the attacking side at root level.
 * @param refutations table where to store refutations
 * @param si slice index
 * @param maximum number of refutations to be delivered
 * @return attacker_has_reached_deadend if we are in a situation where
 *              the position after the attacking move is to be
 *              considered hopeless for the attacker, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *            if there are >max_number_refutations refutations
 *         attacker_has_solved_next_slice if the attacking move has
 *            solved the branch
 *         found_refutations iff refutations contains some refutations
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
slice_root_find_refutations(table refutations,
                            slice_index si,
                            unsigned int max_number_refutations)
{
  quantity_of_refutations_type result = attacker_has_reached_deadend;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",max_number_refutations);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafForced:
      result = leaf_forced_root_find_refutations(refutations,
                                                 si,
                                                 max_number_refutations);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(quantity_of_refutations_type,result,"");
  TraceFunctionResultEnd();
  return result;
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
    case STDirectDefenderRoot:
      result = branch_d_defender_root_make_setplay_slice(si);
      break;

    case STHelpRoot:
      result = help_root_make_setplay_slice(si);
      break;

    case STSeriesRoot:
      result = series_root_make_setplay_slice(si);
      break;

    case STMoveInverter:
      result = move_inverter_root_make_setplay_slice(si);
      break;

    case STSelfAttack:
      result = self_attack_root_make_setplay_slice(si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_make_setplay_slice(si);
      break;

    case STDirectDefense:
    case STDirectRoot:
    case STSelfCheckGuard:
    case STRestartGuard:
    case STMaxFlightsquares:
    case STKeepMatingGuard:
    case STMaxNrNonTrivial:
    case STMaxThreatLength:
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

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @return index of first postkey slice; no_slice if postkey play not
 *         applicable
 */
slice_index slice_root_reduce_to_postkey_play(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectRoot:
      result = direct_root_reduce_to_postkey_play(si);
      break;

    case STDirectDefenderRoot:
      result = branch_d_defender_root_reduce_to_postkey_play(si);
      break;

    case STDirectDefense:
      result = direct_defense_root_reduce_to_postkey_play(si);
      break;

    case STSelfAttack:
      result = self_attack_root_reduce_to_postkey_play(si);
      break;

    case STReflexGuard:
      result = reflex_guard_root_reduce_to_postkey_play(si);
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
    case STDirectHashed:
    case STDirectDefense:
    case STSelfDefense:
    case STReflexGuard:
    case STDegenerateTree:
    case STLeafDirect:
      solution_found = direct_solve(si);
      break;

    case STBranchHelp:
    case STHelpHashed:
      solution_found = help_solve(si);
      break;

    case STBranchSeries:
    case STSeriesHashed:
      solution_found = series_solve(si);
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

/* As slice_solve(), but the key move has just been played.
 * I.e. determine whether a slice has been solved with the move just
 * played; if yes, write the solution including the move just played.
 * @param si slice identifier
 * @return true iff the slice is solved
 */
boolean slice_solved(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafForced:
      result = leaf_forced_solved(si);
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
    case STDirectDefenderRoot:
    case STLeafDirect:
    case STDirectDefense:
    case STSelfDefense:
    case STDirectHashed:
    case STMaxThreatLength:
      result = direct_root_solve(si);
      break;

    case STHelpRoot:
    case STBranchHelp:
    case STHelpHashed:
    case STLeafHelp:
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

    case STDirectHashed:
      result = slice_are_threats_refuted(threats,slices[si].u.pipe.next);
      break;

    case STReciprocal:
      result = reci_are_threats_refuted(threats,si);
      break;

    case STQuodlibet:
      result = quodlibet_are_threats_refuted(threats,si);
      break;

    case STHelpHashed:
      result = help_are_threats_refuted(threats,si);
      break;

    case STBranchHelp:
      result = branch_h_are_threats_refuted(threats,si);
      break;

    case STBranchSeries:
    case STSeriesHashed:
      result = series_are_threats_refuted(threats,si);
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
  has_solution_type result = has_no_solution;

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
    case STDirectDefense:
    case STSelfDefense:
    case STReflexGuard:
    case STKeepMatingGuard:
    case STDegenerateTree:
    case STDirectHashed:
      result = direct_has_solution(si);
      break;

    case STHelpRoot:
      result = help_root_has_solution(si);
      break;

    case STBranchHelp:
      result = help_has_solution(si);
      break;

    case STHelpHashed:
      result = slice_has_solution(slices[si].u.pipe.next);
      break;

    case STBranchSeries:
    case STSeriesHashed:
      result = series_has_solution(si);

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

    case STBranchHelp:
      result = help_solve_postkey(si);
      break;

    case STHelpHashed:
      result = slice_solve_postkey(slices[si].u.pipe.next);
      break;

    case STBranchSeries:
    case STSeriesHashed:
      result = series_solve_postkey(si);
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
    case STLeafForced:
      result = leaf_forced_has_non_starter_solved(si);
      break;

    case STBranchDirect:
      result = branch_d_has_non_starter_solved(si);
      break;

    case STBranchHelp:
    case STHelpHashed:
      result = help_has_non_starter_solved(si);
      break;

    case STBranchSeries:
    case STSeriesHashed:
      result = series_has_non_starter_solved(si);
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

/* Determine whether there are refutations
 * @param leaf slice index
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int slice_count_refutations(slice_index si,
                                     unsigned int max_result)
{
  unsigned int result = max_result+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafForced:
      result = leaf_forced_count_refutations(si,max_result);
      break;

    case STQuodlibet:
      result = quodlibet_count_refutations(si,max_result);
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

    case STDirectHashed:
    case STHelpHashed:
      result = slice_is_goal_reached(just_moved,slices[si].u.pipe.next);
      break;

    case STDirectDefenderRoot:
    case STBranchDirectDefender:
      result = branch_d_defender_is_goal_reached(just_moved,si);
      break;

    case STHelpRoot:
      result = help_is_goal_reached(just_moved,si);
      break;

    case STSeriesRoot:
    case STBranchSeries:
    case STSeriesHashed:
      result = series_is_goal_reached(just_moved,si);
      break;

    case STBranchHelp:
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
    case STReflexGuard:
    case STDirectDefense:
    case STSelfDefense:
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
