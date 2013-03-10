#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/solve.h"
#include "debugging/trace.h"

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

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies solve adapter slice
 * @param st address of structure holding the traversal state
 */
void defense_adapter_make_root(slice_index adapter,
                               stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  battle_make_root(adapter,state);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Wrap the slices representing the nested slices
 * @param adapter identifies solve adapter slice
 * @param st address of structure holding the traversal state
 */
void defense_adapter_make_intro(slice_index adapter,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(adapter,st);

  if (st->level==structure_traversal_level_nested
      && slices[adapter].u.branch.length>slack_length)
  {
    spin_off_state_type * const state = st->param;
    battle_spin_off_intro(adapter,state);
  }

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
stip_length_type defense_adapter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type defense_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  defense_result = solve(next,length);
  if (defense_result<slack_length)
    result = immobility_on_next_move;
  else if (defense_result<=length)
    result = n;
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
