#include "pydirect.h"
#include "pybrad.h"
#include "pybradd.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pyint.h"
#include "trace.h"

#include <assert.h>

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff >= 1 solution has been found
 */
boolean direct_has_solution_in_n(slice_index si,
                                 stip_length_type n,
                                 int curr_max_nr_nontrivial)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirect:
      result = branch_d_has_solution_in_n(si,n,curr_max_nr_nontrivial);
      break;

    case STBranchFork:
      result = branch_fork_has_solution_in_n(si,n,curr_max_nr_nontrivial);
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
void direct_solve_continuations_in_n(table continuations,
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
    case STBranchDirect:
      branch_d_solve_continuations_in_n(continuations,si,n);
      break;

    case STBranchFork:
      branch_fork_solve_continuations_in_n(continuations,si,n);
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
 * @param n (odd) number of half moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff defender wins
 */
boolean direct_defender_does_defender_win(slice_index si,
                                          stip_length_type n,
                                          int curr_max_nr_nontrivial)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = branch_d_defender_does_defender_win(si,
                                                   n,
                                                   curr_max_nr_nontrivial);
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

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n (odd) number of half moves until goal
 * @return true iff >=1 solution was found
 */
boolean direct_defender_solve_postkey_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBranchDirectDefender:
      result = branch_d_defender_solve_postkey_in_n(si,n);
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

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 */
/* TODO get rid of this */
void direct_defender_root_solve_postkey(table refutations, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot:
      branch_d_defender_root_solve_postkey(refutations,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find refutations after a move of the attacking side at root level.
 * @param t table where to store refutations
 * @param si slice index
 * @return max_nr_refutations+1 if
 *            if the defending side is immobile (it shouldn't be here!)
 *            if the defending side has more non-trivial moves than allowed
 *            if the defending king has more flights than allowed
 *            if there is no threat in <= the maximal threat length
 *               as entered by the user
 *         number (0..max_nr_refutations) of refutations otherwise
 */
unsigned int direct_defender_root_find_refutations(table refutations,
                                                   slice_index si)
{
  unsigned int result = UINT_MAX;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STDirectDefenderRoot:
      result = branch_d_defender_root_find_refutations(refutations,si);
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
