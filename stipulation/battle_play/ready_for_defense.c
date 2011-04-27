#include "stipulation/battle_play/ready_for_defense.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/help_play/adapter.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STReadyForDefense defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_defense_slice(stip_length_type length,
                                          stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STReadyForDefense,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void ready_for_defense_make_setplay_slice(slice_index si,
                                          stip_structure_traversal *st)
{
  slice_index * const result = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(length>=slack_length_battle);

  if (length==slack_length_battle)
  {
    slice_index const adapter = alloc_defense_adapter_slice(length,min_length);
    pipe_link(adapter,si);
    *result = adapter;
  }
  else
  {
    {
      slice_index const adapter = alloc_attack_adapter_slice(length-1,
                                                             min_length-1);
      slice_index const end = branch_find_slice(STEndOfRoot,si);
      assert(end!=no_slice);
      pipe_set_successor(adapter,end);
      *result = adapter;
    }

    stip_traverse_structure_children(si,st);

    {
      stip_length_type const length_h = slack_length_help+1;
      slice_index const adapter = alloc_help_adapter_slice(length_h,
                                                           length_h);
      slice_index const ready = alloc_branch(STReadyForHelpMove,
                                             length_h,length_h);
      pipe_link(adapter,ready);
      link_to_branch(ready,*result);
      *result = adapter;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void ready_for_defense_apply_postkeyplay(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index * const postkey_slice = st->param;
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const adapter = alloc_defense_adapter_slice(length,min_length);
    pipe_set_successor(adapter,si);
    *postkey_slice = adapter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
