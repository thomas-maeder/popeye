#include "pieces/attributes/paralysing/mate_filter.h"
#include "pypipe.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STPiecesParalysingMateFilter
 */

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_mate_filter_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingMateFilter);
  slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STPiecesParalysingMateFilterTester slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_mate_filter_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingMateFilterTester);
  slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_mate_filter_tester_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const mated = (slices[si].u.goal_filter.applies_to_who
                      ==goal_applies_to_starter
                      ? slices[si].starter
                      : advers(slices[si].starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(next);
  if (result==has_solution && suffocated_by_paralysis(mated))
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_mate_filter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  Side const mated = (slices[si].u.goal_filter.applies_to_who
                      ==goal_applies_to_starter
                      ? slices[si].starter
                      : advers(slices[si].starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (suffocated_by_paralysis(mated))
    result = has_no_solution;
  else
    result = slice_solve(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
