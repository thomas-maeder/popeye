#include "stipulation/end_of_branch_goal.h"
#include "stipulation/stipulation.h"
#include "stipulation/fork.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STEndOfBranchGoal slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_goal(slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_fork_slice(STEndOfBranchGoal,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type end_of_branch_goal_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const op1 = slices[si].next1;
  slice_index const op2 = slices[si].next2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(op2,n);
  if (result>n)
    result = solve(op1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
