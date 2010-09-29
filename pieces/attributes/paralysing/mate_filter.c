#include "pieces/attributes/paralysing/mate_filter.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STPiecesParalysingMateFilter
 */

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param side side to be mated
 * @return index of allocated slice
 */
slice_index alloc_paralysing_mate_filter_slice(Side side)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingMateFilter);
  slices[result].u.goal_filter.goaled = side;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean has_move(Side side)
{
  boolean result;
  move_generation_mode = move_generation_not_optimized;
  genmove(side);
  result = encore();
  finply();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_mate_filter_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (has_move(slices[si].u.goal_filter.goaled))
    result = slice_has_solution(next);
  else
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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (has_move(slices[si].u.goal_filter.goaled))
    result = slice_solve(next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
