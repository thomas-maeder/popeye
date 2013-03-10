#include "stipulation/help_play/adapter.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "solving/solve.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

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

/* Wrap the slices representing the nested slices
 * @param adapter identifies solve adapter slice
 * @param st address of structure holding the traversal state
 */
void help_adapter_make_intro(slice_index adapter, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(adapter,st);

  if (st->level==structure_traversal_level_nested
      && slices[adapter].u.branch.length>slack_length)
  {
    spin_off_state_type * const state = st->param;
    help_spin_off_intro(adapter,state);
  }

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type help_adapter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type const full_length = slices[si].u.branch.length;
  slice_index const next = slices[si].next1;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nr_moves_needed = solve(next,full_length);
  if (nr_moves_needed<slack_length)
    result = immobility_on_next_move;
  else if (nr_moves_needed<=full_length)
    result = n;
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
