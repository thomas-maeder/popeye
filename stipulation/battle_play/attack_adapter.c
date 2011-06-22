#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "pypipe.h"
#include "trace.h"

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
 * @param adapter identifies attack adapter slice
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
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void attack_adapter_make_intro(slice_index adapter,
                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(adapter,st);

  if (st->level==structure_traversal_level_nested
      /* this filters out adapters that are not in a loop
       * TODO  should we get rid of these? */
      && branch_find_slice(STAttackAdapter,slices[adapter].u.pipe.next)==adapter)
  {
    spin_off_state_type * const state = st->param;
    battle_spin_off_intro(adapter,state);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_attack_adpater(slice_index si,
                                            stip_structure_traversal *st)
{
  structure_traversal_level_type const save_level = st->level;
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->context = stip_traversal_context_attack;
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
void stip_traverse_moves_attack_adapter(slice_index si,
                                        stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_global)
  {
    assert(st->remaining==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED);
    assert(st->full_length==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED);
    st->full_length = slices[si].u.branch.length-slack_length_battle;
    TraceValue("->%u\n",st->full_length);
    st->remaining = st->full_length;
    st->context = stip_traversal_context_attack;

    stip_traverse_moves_pipe(si,st);

    st->context = stip_traversal_context_global;
    st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
    st->full_length = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  }
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Attempt to add set play to an attack stipulation (battle play, not
 * postkey only)
 * @param adapter identifies attack adapter slice
 * @param st address of structure representing traversal
 */
void attack_adapter_apply_setplay(slice_index adapter, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;
  stip_length_type const length = slices[adapter].u.branch.length;
  stip_length_type const min_length = slices[adapter].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  if (length>slack_length_battle)
  {
    slice_index zigzag = branch_find_slice(STCheckZigzagJump,adapter);
    if (zigzag==no_slice)
    {
      battle_branch_make_setplay(adapter,state);
      if (state->spun_off[adapter]!=no_slice)
      {
        slice_index const nested = state->spun_off[adapter];
        state->spun_off[adapter] = alloc_defense_adapter_slice(length-1,min_length-1);
        link_to_branch(state->spun_off[adapter],nested);
      }
    }
    else
    {
      /* set play of some pser stipulation */
      slice_index const proto = alloc_defense_adapter_slice(slack_length_battle,
                                                            slack_length_battle);
      battle_branch_insert_slices(adapter,&proto,1);

      {
        slice_index const defense_adapter = branch_find_slice(STDefenseAdapter,
                                                              adapter);
        assert(defense_adapter!=no_slice);
        battle_branch_make_root_slices(defense_adapter,state);
        assert(state->spun_off[defense_adapter]!=no_slice);
        state->spun_off[adapter] = state->spun_off[defense_adapter];
        pipe_remove(defense_adapter);
      }
    }
  }

  TraceValue("%u\n",state->spun_off[adapter]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type attack_adapter_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves_needed = can_attack(next,length,min_length-1);
  if (nr_moves_needed<slack_length_battle)
    result = opponent_self_check;
  else if (nr_moves_needed<=length)
    result = has_solution;
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type attack_adapter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type nr_moves_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves_needed = attack(next,length,min_length-1);
  if (nr_moves_needed==slack_length_battle-2)
    result = opponent_self_check;
  else if (nr_moves_needed<=length)
    result = has_solution;
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_ready_for_attack(slice_index si,
                                              stip_structure_traversal *st)
{
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->context!=stip_traversal_context_global);

  st->context = stip_traversal_context_attack;
  stip_traverse_structure_pipe(si,st);
  st->context = save_context;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves of some adapter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_ready_for_attack(slice_index si,
                                          stip_moves_traversal *st)
{
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",st->context);
  assert(st->context!=stip_traversal_context_global);

  st->context = stip_traversal_context_attack;
  stip_traverse_moves_pipe(si,st);
  st->context = save_context;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
