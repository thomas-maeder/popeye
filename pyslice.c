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

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean slice_root_defend(slice_index si, unsigned int max_number_refutations)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",max_number_refutations);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafForced:
      result = leaf_forced_root_defend(si,max_number_refutations);
      break;

    case STQuodlibet:
      result = quodlibet_root_defend(si,max_number_refutations);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
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
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean slice_are_threats_refuted(table threats,
                                  stip_length_type len_threat,
                                  slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectHashed:
    case STLeafDirect:
      result = direct_are_threats_refuted(threats,len_threat,si);
      break;

    case STBranchHelp:
    case STHelpHashed:
      result = help_are_threats_refuted(threats,len_threat,si);
      break;

    case STBranchSeries:
    case STSeriesHashed:
      result = series_are_threats_refuted(threats,len_threat,si);
      break;

    case STReciprocal:
      result = reci_are_threats_refuted(threats,len_threat,si);
      break;

    case STQuodlibet:
      result = quodlibet_are_threats_refuted(threats,len_threat,si);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_are_threats_refuted(threats,len_threat,si);
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

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean slice_defend(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafForced:
      result = leaf_forced_defend(si);
      break;

    case STLeafHelp:
      result = leaf_h_defend(si);
      break;

    case STQuodlibet:
      result = quodlibet_defend(si);
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
