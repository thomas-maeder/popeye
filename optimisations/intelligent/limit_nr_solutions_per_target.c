#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "pydata.h"
#include "pypipe.h"
#include "pyint.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Allocate a STIntelligentSolutionsPerTargetPosCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_nr_solutions_per_target_position_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentSolutionsPerTargetPosCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
intelligent_nr_solutions_per_target_position_counter_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
intelligent_nr_solutions_per_target_position_counter_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_solve(slices[si].u.pipe.next);
  if (result==has_solution)
    ++sol_per_matingpos;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}


/* Allocate a STIntelligentLimitNrSolutionsPerTargetPos slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_limit_nr_solutions_per_target_position_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentLimitNrSolutionsPerTargetPos);

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
 */
stip_length_type
intelligent_limit_nr_solutions_per_target_position_can_help(slice_index si,
                                                            stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_help(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
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
 */
stip_length_type
intelligent_limit_nr_solutions_per_target_position_help(slice_index si,
                                                        stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sol_per_matingpos>=maxsol_per_matingpos)
  {
    FlagMaxSolsPerMatingPosReached = true;
    result = n+2;
  }
  else
    result = help(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
