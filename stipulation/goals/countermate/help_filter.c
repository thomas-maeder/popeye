#include "stipulation/goals/countermate/help_filter.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STCounterMateHelpFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_countermate_help_filter_slice(stip_length_type length,
                                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STCounterMateHelpFilter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type countermate_help_filter_solve_in_n(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if (goal_checker_mate(advers(slices[si].starter))==goal_reached)
    result = help_solve_in_n(slices[si].u.pipe.next,slack_length_help+1);
  else
    result = slack_length_help+3;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type countermate_help_filter_has_solution_in_n(slice_index si,
                                                           stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_help+1);

  if (goal_checker_mate(advers(slices[si].starter))==goal_reached)
    result = help_has_solution_in_n(slices[si].u.pipe.next,
                                    slack_length_help+1);
  else
    result = slack_length_help+3;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
