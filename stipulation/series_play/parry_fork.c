#include "stipulation/series_play/parry_fork.h"
#include "pydata.h"
#include "pyproc.h"
#include "pybrafrk.h"
#include "stipulation/series_play/play.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Determine and write the solution(s)
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type parry_fork_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,side_at_move))
    result = series(slices[si].u.parry_fork.next,n);
  else
    result = series(slices[si].u.parry_fork.non_parrying,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type parry_fork_has_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const side_at_move = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,side_at_move))
    result = has_series(slices[si].u.parry_fork.next,n);
  else
    result = has_series(slices[si].u.parry_fork.non_parrying,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STParryFork slice.
 * @param parrying identifies slice responsible for parrying
 * @return allocated slice
 */
static slice_index alloc_parry_fork(slice_index non_parrying)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",non_parrying);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STParryFork,non_parrying);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Convert a series branch to a parry series branch
 * @param si identifies first slice of the series branch
 * @param non_parrying identifies slice responsible for parrying
 */
void convert_to_parry_series_branch(slice_index si, slice_index parrying)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",parrying);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  {
    slice_index const ready = branch_find_slice(STReadyForSeriesDummyMove,si);
    slice_index const parry_fork = alloc_parry_fork(ready);

    assert(ready!=no_slice);

    pipe_link(slices[ready].prev,parry_fork);
    pipe_link(parry_fork,parrying);

    slice_set_predecessor(ready,no_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_parry_fork(slice_index branch,
                                        stip_structure_traversal *st)
{
  slice_index const non_parrying = slices[branch].u.parry_fork.non_parrying;
  stip_traverse_structure_pipe(branch,st);
  stip_traverse_structure(non_parrying,st);
}

/* Traversal of the moves of some pipe slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_parry_fork(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
