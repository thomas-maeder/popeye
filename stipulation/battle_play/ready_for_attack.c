#include "stipulation/battle_play/ready_for_attack.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STReadyForAttack slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_attack_slice(stip_length_type length,
                                         stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STReadyForAttack,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void ready_for_attack_reduce_to_postkey_play(slice_index si,
                                             stip_structure_traversal *st)
{
  slice_index const *postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  if (*postkey_slice!=no_slice)
    dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void ready_for_attack_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const copy = copy_slice(si);
    link_to_branch(copy,*root_slice);
    *root_slice = copy;
  }

  battle_branch_shorten_slice(si);

  if (slices[si].u.branch.min_length>slack_length_battle+1)
  {
    slice_index const root_attack_fork = branch_find_slice(STRootAttackFork,
                                                           *root_slice);
    if (root_attack_fork!=no_slice)
    {
      dealloc_slices(slices[root_attack_fork].u.branch_fork.towards_goal);
      pipe_remove(root_attack_fork);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
