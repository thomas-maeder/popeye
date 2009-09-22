#include "pyhelp.h"
#include "pybrah.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pyint.h"
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
