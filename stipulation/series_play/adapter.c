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

  *root_slice = series_make_root(adapter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Wrap the slices representing the nested slices
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void series_adapter_make_intro(slice_index adapter,
                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(adapter,st);

  if (st->level==structure_traversal_level_nested)
    series_spin_off_intro(adapter);

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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_series_adpater(slice_index si,
                                            stip_structure_traversal *st)
{
  structure_traversal_level_type const save_level = st->level;
  structure_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->context = structure_traversal_context_series;
  st->level = structure_traversal_level_nested;
  stip_traverse_structure_pipe(si,st);
  st->level = save_level;
  st->context = save_context;

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
  if (nr_moves_needed<=full_length)
    result = has_solution;
  else if (nr_moves_needed==full_length+1)
    result = has_no_solution;
  else
    result = opponent_self_check;

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
  if (nr_moves_needed<=full_length)
    result = has_solution;
  else if (nr_moves_needed==full_length+1)
    result = has_no_solution;
  else
    result = opponent_self_check;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
