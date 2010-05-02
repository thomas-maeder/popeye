#include "optimisations/maxsolutions/help_filter.h"
#include "optimisations/maxsolutions/maxsolutions.h"
#include "stipulation/help_play/play.h"
#include "pypipe.h"
#include "trace.h"

/* Allocate a STMaxSolutionsHelpFilter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_help_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMaxSolutionsHelpFilter); 

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 *         n-2 the previous move has solved the next slice
 */
stip_length_type maxsolutions_help_filter_solve_in_n(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (max_nr_solutions_found_in_phase())
    result = n+2;
  else
    result = help_solve_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 *         n-2 the previous move has solved the next slice
 */
stip_length_type
maxsolutions_help_filter_has_solution_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = help_has_solution_in_n(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
