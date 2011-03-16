#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
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

/* Traversal of the moves of some adapter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_battle_adapter_slice(slice_index si,
                                              stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED)
  {
    stip_length_type const save_full_length = st->full_length;
    st->full_length = slices[si].u.branch.length-slack_length_battle;
    TraceValue("->%u\n",st->full_length);
    st->remaining = st->full_length;
    stip_traverse_moves_pipe(si,st);
    st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
    st->full_length = save_full_length;
  }
  else
    stip_traverse_moves_pipe(si,st);

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

  nr_moves_needed = attack_has_solution_in_n(next,length,min_length-1);
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

  nr_moves_needed = attack_solve_in_n(next,length,min_length-1);
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
