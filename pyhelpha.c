#include "pyhelpha.h"
#include "pybrah.h"
#include "pyhash.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STHelpHashed slice for a STBranchHelp slice
 * @param base identifies STBranchHelp slice
 * @return index of allocated slice
 */
slice_index alloc_help_hashed_slice(slice_index base)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  slices[result].type = STHelpHashed;
  slices[result].starter = slices[base].starter;
  slices[result].u.pipe.u.branch = slices[base].u.pipe.u.branch;
  slices[result].u.pipe.next = base;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at the move
 * @return true iff >=1 solution was found
 */
boolean help_hashed_solve_in_n(slice_index si,
                               stip_length_type n,
                               Side side_at_move)
{
  hashwhat const hash_no_succ = n%2==0 ? HelpNoSuccEven : HelpNoSuccOdd;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (inhash(si,hash_no_succ,n/2))
    result = false;
  else if (help_solve_in_n(slices[si].u.pipe.next,n,side_at_move))
    result = true;
  else
  {
    result = false;
    addtohash(si,hash_no_succ,n/2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
boolean help_hashed_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      Side side_at_move)
{
  hashwhat const hash_no_succ = n%2==0 ? HelpNoSuccEven : HelpNoSuccOdd;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (inhash(si,hash_no_succ,n/2))
    result = false;
  else
  {
    if (help_has_solution_in_n(slices[si].u.pipe.next,n,side_at_move))
      result = true;
    else
    {
      addtohash(si,hash_no_succ,n/2);
      result = false;
    }
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
 * @param side_at_move side at move
 */
void help_hashed_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n,
                                          Side side_at_move)
{
  hashwhat const hash_no_succ = n%2==0 ? HelpNoSuccEven : HelpNoSuccOdd;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",side_at_move);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (!inhash(si,hash_no_succ,n/2))
  {
    slice_index const next = slices[si].u.pipe.next;
    help_solve_continuations_in_n(continuations,next,n,side_at_move);
    if (table_length(continuations)==0)
      addtohash(si,hash_no_succ,n/2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine the starter in a help stipulation in n half-moves
 * @param si identifies slice
 * @param n number of half-moves
 * @param return starting side
 */
Side help_hashed_starter_in_n(slice_index si, stip_length_type n)
{
  Side result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = help_starter_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
