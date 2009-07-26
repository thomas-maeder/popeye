#include "pyhelp.h"
#include "pybrah.h"
#include "pybrafrk.h"
#include "pyhelpha.h"
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

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STBranchHelp:
      result = branch_h_solve_in_n(si,n);
      break;

    case STBranchFork:
      result = branch_fork_help_solve_in_n(si,n);
      break;

    case STHelpHashed:
      result = help_hashed_solve_in_n(si,n);
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

  TraceValue("%u\n",slices[si].type);
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
      result = help_hashed_has_solution_in_n(si,n);
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

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STBranchHelp:
      branch_h_solve_continuations_in_n(continuations,si,n);
      break;

    case STBranchFork:
      branch_fork_help_solve_continuations_in_n(continuations,si,n);
      break;

    case STHelpHashed:
      help_hashed_solve_continuations_in_n(continuations,si,n);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean help_must_starter_resign(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slices[si].type);
  switch (slices[si].type)
  {
    case STBranchHelp:
    case STHelpHashed:
      result = branch_h_must_starter_resign(si);
      break;

    case STBranchFork:
      result = branch_fork_must_starter_resign(si);
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
