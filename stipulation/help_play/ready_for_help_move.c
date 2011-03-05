#include "stipulation/help_play/ready_for_help_move.h"
#include "trace.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/adapter.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/shortcut.h"

#include <assert.h>

/* Allocate a STReadyForHelpMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_help_move_slice(stip_length_type length,
                                            stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STReadyForHelpMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void ready_for_help_move_make_root(slice_index si,
                                   stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;
  slice_index copy;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  copy = copy_slice(si);
  pipe_link(copy,*root_slice);
  *root_slice = copy;
  help_branch_shorten_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void ready_for_help_move_make_setplay_slice(slice_index si,
                                            stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(length>=slack_length_help+1);

  if (min_length<=slack_length_help)
    min_length += 2;

  if (length==slack_length_help+1)
  {
    *setplay_slice = alloc_help_adapter_slice(length,min_length);
    pipe_set_successor(*setplay_slice,si);
  }
  else
  {
    slice_index const adapter = alloc_help_adapter_slice(length,min_length);
    slice_index const root = alloc_help_root_slice(length,min_length);
    slice_index const shortcut = alloc_help_shortcut(length,min_length,si);
    slice_index const copy = alloc_ready_for_help_move_slice(length,min_length);

    stip_traverse_structure_children(si,st);

    pipe_link(adapter,root);
    pipe_link(root,shortcut);
    pipe_link(shortcut,copy);
    pipe_link(copy,*setplay_slice);
    *setplay_slice = adapter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
