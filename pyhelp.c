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
 * @param n number of half moves until end state has to be reached
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
 * @param n number of half moves until end state has to be reached
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

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void help_solve_continuations_in_n(table continuations,
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
    case STBranchHelp:
      branch_h_solve_continuations_in_n(continuations,si,n);
      break;

    case STBranchFork:
      branch_fork_help_solve_continuations_in_n(continuations,si,n);
      break;

    case STHelpHashed:
      hashed_help_solve_continuations_in_n(continuations,si,n);
      break;

    case STReflexGuard:
      reflex_guard_help_solve_continuations_in_n(continuations,si,n);
      break;

    case STKeepMatingGuard:
      keepmating_guard_help_solve_continuations_in_n(continuations,si,n);
      break;

    case STGoalReachableGuard:
      goalreachable_guard_help_solve_continuations_in_n(continuations,si,n);
      break;

    case STSelfCheckGuard:
      selfcheck_guard_help_solve_continuations_in_n(continuations,si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
