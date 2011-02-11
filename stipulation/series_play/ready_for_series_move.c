#include "stipulation/series_play/ready_for_series_move.h"
#include "pypipe.h"
#include "trace.h"
#include "stipulation/branch.h"
#include "stipulation/series_play/adapter.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/shortcut.h"

#include <assert.h>

/* Allocate a STReadyForSeriesMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_series_move_slice(stip_length_type length,
                                              stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STReadyForSeriesMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void ready_for_series_move_make_root(slice_index si,
                                     stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  slice_index ready;
  slice_index adapter;
  slice_index new_root;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ready = alloc_ready_for_series_move_slice(length,min_length);
  adapter = alloc_series_adapter_slice(length,min_length);
  new_root = alloc_series_root_slice(length,min_length);
  pipe_link(ready,adapter);
  pipe_link(adapter,new_root);

  if (length<slack_length_series+2)
    pipe_set_successor(new_root,slices[si].u.pipe.next);
  else
  {
    slice_index const shortcut = alloc_series_shortcut(length,min_length,si);
    pipe_link(new_root,shortcut);
    stip_traverse_structure_children(si,st);
    assert(state->result!=no_slice);
    pipe_link(shortcut,state->result);
    shorten_series_pipe(si);
  }

  state->result = ready;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
