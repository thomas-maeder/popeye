#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/branch.h"
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

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void defense_adapter_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].prev==si)
  {
    slice_index const copy = copy_slice(si);
    stip_traverse_structure_children(si,st);
    link_to_branch(copy,*root_slice);
    *root_slice = copy;
  }
  else
    slice_move_to_root(si,st);

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

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type defense_adapter_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.branch.next;
  stip_length_type const n_battle = (n+slack_length_battle
                                     -slack_length_series);
  stip_length_type const n_max_unsolvable = slack_length_battle-1;
  stip_length_type defense_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  defense_result = defend(next,n_battle,n_max_unsolvable);
  if (slack_length_battle<=defense_result && defense_result<=n_battle)
    result = defense_result+slack_length_series-slack_length_battle;
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type defense_adapter_has_series(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.branch.next;
  stip_length_type const n_battle = (n+slack_length_battle
                                     -slack_length_series);
  stip_length_type const n_max_unsolvable = slack_length_battle-1;
  stip_length_type defense_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  defense_result = can_defend(next,n_battle,n_max_unsolvable);
  if (slack_length_battle<=defense_result && defense_result<=n_battle)
    result = defense_result+slack_length_series-slack_length_battle;
  else
    result = n+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
