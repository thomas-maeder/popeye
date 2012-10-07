#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STAttackAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_adapter_slice(stip_length_type length,
                                       stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STAttackAdapter,length,min_length);

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
void attack_adapter_make_root(slice_index adapter,
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
void attack_adapter_make_intro(slice_index adapter,
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

/* Apply setplay to a branch with alternate moves
 * @param adapter STAttackAdapter slice
 * @param st holds the setplay application traversal
 */
static void apply_setplay_alternate(slice_index adapter,
                                    stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  battle_branch_make_setplay(adapter,state);

  if (state->spun_off[adapter]!=no_slice)
  {
    stip_length_type const length = slices[adapter].u.branch.length;
    stip_length_type const min_length = slices[adapter].u.branch.min_length;
    slice_index const nested = state->spun_off[adapter];
    state->spun_off[adapter] = alloc_defense_adapter_slice(length-1,
                                                           min_length-1);
    link_to_branch(state->spun_off[adapter],nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Apply setplay to a series branch
 * @param adapter STAttackAdapter slice
 * @param st holds the setplay application traversal
 */
static void apply_setplay_series(slice_index adapter,
                                 stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    slice_index const proto = alloc_defense_adapter_slice(slack_length,
                                                          slack_length);
    branch_insert_slices(adapter,&proto,1);
  }

  {
    slice_index const defense_adapter = branch_find_slice(STDefenseAdapter,
                                                          adapter,
                                                          stip_traversal_context_intro);
    assert(defense_adapter!=no_slice);
    battle_branch_make_root_slices(defense_adapter,state);
    assert(state->spun_off[defense_adapter]!=no_slice);
    state->spun_off[adapter] = state->spun_off[defense_adapter];
    pipe_remove(defense_adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember a defense move in the normal path
 * @param si STMove slice
 * @st holds the defense move finding traversal
 */
static void remember_defense(slice_index si,
                             stip_structure_traversal *st)
{
  boolean * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_defense)
    *result = true;
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is there a defense move in the "normal" path?
 * @param adapter identifies STAttackAdapter slice
 * @param st holds the setplay application traversal
 * @return true iff there is a defense move on the normal path
 */
static boolean find_defense_in_normal_path(slice_index adapter,
                                           stip_structure_traversal *st)
{
  boolean result = false;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init_nested(&st_nested,st,&result);
  branch_instrument_traversal_for_normal_path(&st_nested);
  stip_structure_traversal_override_single(&st_nested,STMove,&remember_defense);
  stip_traverse_structure(adapter,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Attempt to add set play to an solve stipulation (battle play, not
 * postkey only)
 * @param adapter identifies solve adapter slice
 * @param st address of structure representing traversal
 */
void attack_adapter_apply_setplay(slice_index adapter,
                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  if (slices[adapter].u.branch.length>slack_length)
  {
    if (find_defense_in_normal_path(adapter,st))
      apply_setplay_alternate(adapter,st);
    else
      apply_setplay_series(adapter,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_adapter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nr_moves_needed = solve(next,length);
  if (nr_moves_needed<slack_length)
    result = slack_length-2;
  else if (nr_moves_needed<=length)
    result = n;
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
