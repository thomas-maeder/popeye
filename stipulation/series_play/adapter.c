#include "stipulation/series_play/adapter.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/branch.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STSeriesAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_adapter_slice(stip_length_type length,
                                       stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSeriesAdapter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies slice where to start
 * @param st address of structure holding the traversal state
 */
void series_adapter_make_root(slice_index adapter, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  *root_slice = series_make_root(slices[adapter].u.pipe.next);

  if (*root_slice!=no_slice)
  {
    pipe_link(adapter,*root_slice);
    *root_slice = adapter;
    pipe_unlink(slices[adapter].prev);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Attempt to add set play to an attack stipulation (battle play, not
 * postkey only)
 * @param adapter identifies the root from which to apply set play
 * @param st address of structure representing traversal
 */
void series_adapter_apply_setplay(slice_index adapter, stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  *setplay_slice = series_make_setplay(slices[adapter].u.pipe.next);

  if (*setplay_slice!=no_slice)
  {
    slice_index const set_adapter =
        alloc_series_adapter_slice(slack_length_series,slack_length_series);
    link_to_branch(set_adapter,*setplay_slice);
    *setplay_slice = set_adapter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves of some adapter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_adapter_slice(slice_index si,
                                              stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED)
  {
    stip_length_type const save_full_length = st->full_length;
    st->full_length = slices[si].u.branch.length-slack_length_series;
    TraceValue("->%u\n",st->full_length);
    st->remaining = st->full_length;
    stip_traverse_moves_pipe(si,st);
    st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
    st->full_length = save_full_length;
  }
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_adapter_solve(slice_index si)
{
  has_solution_type result;
  stip_length_type const full_length = slices[si].u.branch.length;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves_needed = series(next,full_length);
  result = nr_moves_needed<=full_length ? has_solution : has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_adapter_has_solution(slice_index si)
{
  has_solution_type result = has_no_solution;
  stip_length_type const full_length = slices[si].u.branch.length;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves_needed = has_series(next,full_length);
  result = nr_moves_needed<=full_length ? has_solution : has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type series_adapter_can_attack(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  stip_length_type const n_ser = n+slack_length_series-slack_length_battle;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  nr_moves_needed = has_series(next,n_ser);
  if (nr_moves_needed==n_ser+2)
    result = slack_length_battle-2;
  else if (nr_moves_needed==n_ser+1)
    result = n+2;
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type series_adapter_attack(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  stip_length_type const n_ser = n+slack_length_series-slack_length_battle;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  nr_moves_needed = series(next,n_ser);
  if (nr_moves_needed==n_ser+2)
    result = slack_length_battle-2;
  else if (nr_moves_needed==n_ser+1)
    result = n+2;
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
