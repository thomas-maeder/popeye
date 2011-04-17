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

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
maxsolutions_root_defender_filter_defend(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (max_nr_solutions_found_in_phase())
    result = n+4;
  else
  {
    result = defend(next,n,n_max_unsolvable);
    if (slack_length_battle<=result && result<=n)
      increase_nr_found_solutions();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
