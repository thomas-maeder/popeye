#include "optimisations/maxsolutions/root_defender_filter.h"
#include "optimisations/maxsolutions/maxsolutions.h"
#include "pypipe.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

/* Allocate a STMaxSolutionsRootDefenderFilter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_root_defender_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMaxSolutionsRootDefenderFilter); 

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defending side can successfully defend
 */
boolean maxsolutions_root_defender_filter_defend(slice_index si,
                                                 stip_length_type n_min)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (max_nr_solutions_found_in_phase()
      || defense_root_defend(slices[si].u.pipe.next,n_min))
    result = true;
  else
  {
    increase_nr_found_solutions();
    result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
