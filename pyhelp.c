#include "pyhelp.h"
#include "pybrah.h"
#include "pyleafh.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pyint.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchHelp:
      result = branch_h_solve_in_n(si,n);
      break;

    case STBranchFork:
      result = branch_fork_help_solve_in_n(si,n);
      break;

    case STHelpHashed:
      result = hashed_help_solve_in_n(si,n);
      break;

    case STLeafHelp:
      assert(n==slack_length_help+1);
      result = leaf_h_solve(si);
      break;

    case STReflexGuard:
      result = reflex_guard_help_solve_in_n(si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_help_solve_in_n(si,n);
      break;

    case STGoalReachableGuard:
      result = goalreachable_guard_help_solve_in_n(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_help_solve_in_n(si,n);
      break;

    case STRestartGuard:
      result = restart_guard_help_solve_in_n(si,n);
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
boolean help_root_solve(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafHelp:
      result = leaf_h_root_solve(si);
      break;

    case STHelpRoot:
      result = help_root_root_solve(si);
      break;

    case STBranchHelp:
      result = help_solve(si);
      break;

    case STHelpHashed:
      result = hashed_help_root_solve(si);
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

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean help_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>slack_length_help);

  while (len<full_length && !result)
  {
    if (help_solve_in_n(si,len))
    {
      result = true;
      FlagShortSolsReached = true;
    }

    len += 2;
  }

  result = result || help_solve_in_n(si,full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean help_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchHelp:
    case STHelpRoot:
      result = branch_h_has_solution_in_n(si,n);
      break;

    case STBranchFork:
      result = branch_fork_help_has_solution_in_n(si,n);
      break;

    case STHelpHashed:
      result = hashed_help_has_solution_in_n(si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_help_has_solution_in_n(si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_help_has_solution_in_n(si,n);
      break;

    case STGoalReachableGuard:
      result = goalreachable_guard_help_has_solution_in_n(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_help_has_solution_in_n(si,n);
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

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void help_solve_threats_in_n(table threats, slice_index si, stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchHelp:
      branch_h_solve_threats_in_n(threats,si,n);
      break;

    case STBranchFork:
      branch_fork_help_solve_threats_in_n(threats,si,n);
      break;

    case STHelpHashed:
      hashed_help_solve_threats_in_n(threats,si,n);
      break;

    case STReflexGuard:
      reflex_guard_help_solve_threats_in_n(threats,si,n);
      break;

    case STKeepMatingGuard:
      keepmating_guard_help_solve_threats_in_n(threats,si,n);
      break;

    case STGoalReachableGuard:
      goalreachable_guard_help_solve_threats_in_n(threats,si,n);
      break;

    case STSelfCheckGuard:
      selfcheck_guard_help_solve_threats_in_n(threats,si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void help_solve_threats(table threats, slice_index si)
{
  boolean solution_found = false;
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<full_length && !solution_found)
  {
    help_solve_threats_in_n(threats,si,len);
    if (table_length(threats)>0)
      solution_found = true;

    len += 2;
  }

  if (!solution_found)
    help_solve_threats_in_n(threats,si,full_length);

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
boolean help_are_threats_refuted(table threats,
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
    case STBranchHelp:
      result = branch_h_are_threats_refuted(threats,len_threat,si);
      break;

    case STHelpHashed:
      result = help_are_threats_refuted(threats,len_threat,si);
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
 * @return true iff >=1 solution was found
 */
boolean help_solve_postkey(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].u.pipe.u.branch.length==slack_length_help+1);
  result = slice_solve_postkey(slices[si].u.pipe.u.branch.towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean help_has_non_starter_solved(slice_index si)
{
  slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_non_starter_solved(to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.pipe.u.branch.length;
  stip_length_type len = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_help);

  while (len<=full_length && result==has_no_solution)
  {
    if (help_has_solution_in_n(si,len))
      result = has_solution;

    len += 2;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
