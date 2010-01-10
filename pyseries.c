#include "pyseries.h"
#include "pybraser.h"
#include "pyhelp.h"
#include "pydirect.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pypipe.h"
#include "stipulation/series_play/parry_fork.h"
#include "pyint.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>


/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchSeries:
      result = branch_ser_solve_in_n(si,n);
      break;

    case STBranchHelp:
    {
      stip_length_type const n_help = n-slack_length_series+slack_length_help;
      result = help_solve_in_n(si,n_help);
      break;
    }

    case STBranchDirectDefender:
    {
      stip_length_type const n_dir = n-slack_length_series+slack_length_direct;
      result = !direct_defender_defend_in_n(si,n_dir);
      break;
    }

    case STParryFork:
      result = parry_fork_solve_in_n(si,n);
      break;

    case STBranchFork:
      result = branch_fork_series_solve_in_n(si,n);
      break;

    case STSeriesHashed:
      result = hashed_series_solve_in_n(si,n);
      break;

    case STMoveInverter:
      result = pipe_series_solve_in_n(si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_series_solve_in_n(si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_series_solve_in_n(si,n);
      break;

    case STGoalReachableGuard:
      result = goalreachable_guard_series_solve_in_n(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_series_solve_in_n(si,n);
      break;

    case STRestartGuard:
      result = restart_guard_series_solve_in_n(si,n);
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

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean series_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const full_length = slices[si].u.pipe.u.help_root.length;
  stip_length_type len = slices[si].u.pipe.u.help_root.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>slack_length_series);

  if (len==slack_length_series)
  {
    if (slice_solve(slices[si].u.pipe.u.help_root.towards_goal))
    {
      result = true;
      FlagShortSolsReached = true;
    }
    else
      ++len;
  }
  
  while (len<full_length && !result)
  {
    if (series_solve_in_n(si,len))
    {
      result = true;
      FlagShortSolsReached = true;
    }

    ++len;
  }

  result = result || series_solve_in_n(si,full_length);

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
boolean series_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchSeries:
    case STSeriesRoot:
      result = branch_ser_has_solution_in_n(si,n);
      break;

    case STBranchDirectDefender:
    {
      stip_length_type const n_dir = n-slack_length_series+slack_length_direct;
      result = direct_defender_can_defend_in_n(si,n_dir,0)==0;
      break;
    }

    case STParryFork:
      result = parry_fork_has_solution_in_n(si,n);
      break;

    case STBranchFork:
      result = branch_fork_series_has_solution_in_n(si,n);
      break;

    case STSeriesHashed:
      result = hashed_series_has_solution_in_n(si,n);
      break;

    case STReflexGuard:
      result = reflex_guard_series_has_solution_in_n(si,n);
      break;

    case STKeepMatingGuard:
      result = keepmating_guard_series_has_solution_in_n(si,n);
      break;

    case STGoalReachableGuard:
      result = goalreachable_guard_series_has_solution_in_n(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_series_has_solution_in_n(si,n);
      break;

    case STMoveInverter:
      result = series_has_solution_in_n(slices[si].u.pipe.next,n);
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
has_solution_type series_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.pipe.u.help_root.length;
  stip_length_type len = slices[si].u.pipe.u.help_root.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_series);

  while (len<=full_length && result==has_no_solution)
  {
    result = series_has_solution_in_n(si,len);
    ++len;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void series_solve_threats_in_n(table threats,
                               slice_index si,
                               stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchSeries:
      branch_ser_solve_threats_in_n(threats,si,n);
      break;

    case STBranchFork:
      branch_fork_series_solve_threats_in_n(threats,si,n);
      break;

    case STSeriesHashed:
      hashed_series_solve_threats_in_n(threats,si,n);
      break;

    case STReflexGuard:
      reflex_guard_series_solve_threats_in_n(threats,si,n);
      break;

    case STKeepMatingGuard:
      keepmating_guard_series_solve_threats_in_n(threats,si,n);
      break;

    case STGoalReachableGuard:
      goalreachable_guard_series_solve_threats_in_n(threats,si,n);
      break;

    case STSelfCheckGuard:
      selfcheck_guard_series_solve_threats_in_n(threats,si,n);
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
void series_solve_threats(table threats, slice_index si)
{
  boolean solution_found = false;
  stip_length_type const full_length = slices[si].u.pipe.u.help_root.length;
  stip_length_type len = slices[si].u.pipe.u.help_root.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(full_length>=slack_length_series);

  while (len<full_length && !solution_found)
  {
    series_solve_threats_in_n(threats,si,len);
    if (table_length(threats)>0)
      solution_found = true;

    ++len;
  }

  if (!solution_found)
    series_solve_threats_in_n(threats,si,full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean series_are_threats_refuted(table threats, slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  if (slices[si].type==STBranchSeries)
    result = branch_ser_are_threats_refuted(threats,si);
  else
    result = series_are_threats_refuted(threats,slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
