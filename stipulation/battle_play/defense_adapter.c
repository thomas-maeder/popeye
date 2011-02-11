#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/defense_play.h"
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

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type defense_adapter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.branch.next;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type n_max_unsolvable = slices[si].u.branch.min_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (defense_defend_in_n(next,length,n_max_unsolvable)>length
            ? has_no_solution
            : has_solution);

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
  stip_length_type n_max_unsolvable = slices[si].u.branch.min_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (defense_can_defend_in_n(next,length,n_max_unsolvable)>length
            ? has_no_solution
            : has_solution);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
