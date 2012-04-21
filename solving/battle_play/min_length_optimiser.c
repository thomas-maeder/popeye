#include "solving/battle_play/min_length_optimiser.h"
#include "pystip.h"
#include "stipulation/branch.h"
#include "solving/avoid_unsolvable.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STMinLengthOptimiser defender slice.
 * @return index of allocated slice
 */
slice_index alloc_min_length_optimiser_slice(stip_length_type length,
                                             stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STMinLengthOptimiser,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
min_length_optimiser_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  slice_index const length = slices[si].u.branch.length;
  slice_index const min_length = slices[si].u.branch.min_length;
  slice_index const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (max_unsolvable+length-min_length<n)
  {
    max_unsolvable = n-(length-min_length);
    TraceValue("->%u\n",max_unsolvable);
  }

  result = attack(next,n);

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u\n",max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
