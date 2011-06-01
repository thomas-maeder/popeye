#include "stipulation/battle_play/defense_adapter.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDefenseAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_adapter_slice(stip_length_type length,
                                        stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STDefenseAdapter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_defense_adapter(slice_index si,
                                             stip_structure_traversal *st)
{
  structure_traversal_level_type const save_level = st->level;
  structure_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->level = structure_traversal_level_nested;
  st->context = structure_traversal_context_defense;
  stip_traverse_structure_pipe(si,st);
  st->level = save_level;
  st->context = save_context;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void defense_adapter_make_root(slice_index adapter,
                               stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  *root_slice = battle_make_root(adapter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Wrap the slices representing the nested slices
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void defense_adapter_make_intro(slice_index adapter,
                                stip_structure_traversal *st)
{
  slice_index * const fork_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(adapter,st);

  if (*fork_slice!=no_slice)
    battle_spin_off_intro(adapter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type defense_adapter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.branch.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type n_max_unsolvable = slack_length_battle-1;
  stip_length_type defense_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  defense_result = defend(next,length,n_max_unsolvable);
  result = (slack_length_battle<=defense_result && defense_result<=length
            ? has_solution
            : has_no_solution);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type defense_adapter_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.branch.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type n_max_unsolvable = slack_length_battle-1;
  stip_length_type defense_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  defense_result = can_defend(next,length,n_max_unsolvable);
  result = (slack_length_battle<=defense_result && defense_result<=length
            ? has_solution
            : has_no_solution);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_ready_for_defense(slice_index si,
                                               stip_structure_traversal *st)
{
  structure_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->context = structure_traversal_context_defense;
  stip_traverse_structure_pipe(si,st);
  st->context = save_context;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
