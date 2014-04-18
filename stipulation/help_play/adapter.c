#include "stipulation/help_play/adapter.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STHelpAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_adapter_slice(stip_length_type length,
                                     stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpAdapter,length,min_length);

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
void help_adapter_make_root(slice_index adapter, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  help_make_root(adapter,state);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void count_move_slice(slice_index si, stip_structure_traversal *st)
{
  unsigned int * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++*result;
  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int count_move_slices_in_normal_path(slice_index si,
                                                     stip_structure_traversal *st)
{
  unsigned int result = 0;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init_nested(&st_nested,st,&result);
  branch_instrument_traversal_for_normal_path(&st_nested);
  stip_structure_traversal_override_single(&st_nested,STMove,&count_move_slice);
  stip_traverse_structure(si,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Attempt to add set play to an solve stipulation (battle play, not
 * postkey only)
 * @param si identifies the root from which to apply set play
 * @param st address of structure representing traversal
 */
void help_adapter_apply_setplay(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (count_move_slices_in_normal_path(si,st)==3)
    help_branch_make_setplay(si,state);
  else
    series_branch_make_setplay(si,state);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
