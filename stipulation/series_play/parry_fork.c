#include "parry_fork.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyseries.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Determine and write the solution(s)
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean parry_fork_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,side_at_move))
    result = series_solve_in_n(slices[si].u.pipe.u.parry_fork.parrying,n+1);
  else
    result = series_solve_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean parry_fork_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,side_at_move))
    result = series_has_solution_in_n(slices[si].u.pipe.u.parry_fork.parrying,
                                      n+1);
  else
    result = series_has_solution_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STParryFork slice.
 * @param parrying identifies slice responsible for parrying
 * @return allocated slice
 */
static slice_index alloc_parry_fork(slice_index parrying)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",parrying);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STParryFork); 
  slices[result].u.pipe.u.parry_fork.parrying = parrying;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Convert a series branch to a parry series branch
 * @param si identifies first slice of the series branch
 * @param parrying identifies slice responsible for parrying
 */
void convert_to_parry_series_branch(slice_index si, slice_index parrying)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",parrying);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  {
    slice_index const branch = branch_find_slice(STBranchSeries,si);
    slice_index const inverter = branch_find_slice(STMoveInverter,branch);
    slice_index const next = slices[inverter].u.pipe.next;
    slice_index const prev = slices[inverter].prev;
    slice_index const fork = alloc_parry_fork(parrying);
    slice_index const proxy_to_next = alloc_proxy_pipe();

    assert(inverter!=no_slice);
    assert(slices[next].type==STBranchSeries
           || slices[next].type==STSeriesFork);

    branch_link(prev,fork);
    branch_link(fork,inverter);

    branch_link(inverter,proxy_to_next);
    branch_link(proxy_to_next,next);
    pipe_set_successor(parrying,proxy_to_next);

    if (slices[branch].u.pipe.next==inverter)
      /* if in the playe after the branch, the same side is to move as
       * in the branch (e.g. in s pser-#N), we have to make sure that
       * the other side gets the chance to parry.
       */
      pipe_set_successor(branch,fork);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
