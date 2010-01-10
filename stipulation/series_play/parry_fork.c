#include "parry_fork.h"
#include "pydata.h"
#include "pyproc.h"
#include "pyseries.h"
#include "pybrafrk.h"
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

/* Initialise a slice to be a STParryFork slice.
 * @param si identifies slice to be initialised
 * @param next identifies next slice
 * @param parrying identifies slice responsible for parrying
 */
static void init_parry_fork(slice_index si,
                            slice_index next,
                            slice_index parrying)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParam("%u",parrying);
  TraceFunctionParamListEnd();

  slices[si].type = STParryFork; 
  slices[si].starter = no_side; 
  slices[si].u.pipe.next = next;
  slices[si].u.pipe.u.parry_fork.parrying = parrying;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  {
    slice_index const inverter = branch_find_slice(STMoveInverter,si);
    slice_index const branch = slices[inverter].u.pipe.next;
    assert(inverter!=no_slice);
    assert(slices[branch].type==STBranchSeries
           || slices[branch].type==STBranchFork);
    slices[parrying].u.pipe.next = branch;
    pipe_insert_before(inverter);
    init_parry_fork(inverter,slices[inverter].u.pipe.next,parrying);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
