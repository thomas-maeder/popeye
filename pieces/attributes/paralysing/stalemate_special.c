#include "pieces/attributes/paralysing/stalemate_special.h"
#include "pydata.h"
#include "pypipe.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STPiecesParalysingStalemateSpecial
 */

/* Allocate a STPiecesParalysingStalemateSpecial slice.
 * @param starter_or_adversary is the starter stalemated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_stalemate_special_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingStalemateSpecial);
  slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_stalemate_special_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  goal_applies_to_starter_or_adversary const
    applies_to_who = slices[si].u.goal_filter.applies_to_who;
  Side const starter = slices[si].starter;
  Side const stalemated = (applies_to_who==goal_applies_to_starter
                           ? starter
                           : advers(starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* only flag selfcheck if the side that has just moved is not the one to be
   * stalemated (i.e. if the stipulation is not auto-stalemate) */
  if (applies_to_who==goal_applies_to_starter && echecc(nbply,advers(starter)))
    result = opponent_self_check;
  else if (suffocated_by_paralysis(stalemated))
    result = has_solution;
  else
    result = slice_solve(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
