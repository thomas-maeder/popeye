#include "output/plaintext/goal_writer.h"
#include "stipulation/stipulation.h"
#include "pyproc.h"
#include "stipulation/pipe.h"
#include "output/plaintext/tree/tree.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides the STOutputPlaintextGoalWriter slice type.
 * Slices of this type write the goal at the end of a variation
 */


/* Allocate a STOutputPlaintextGoalWriter slice.
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_goal_writer_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlaintextGoalWriter);
  slices[result].u.goal_handler.goal = goal;

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
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type output_plaintext_goal_writer_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;
  Goal const goal = slices[si].u.goal_handler.goal;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  StdString(goal_end_marker[goal.type]);
  result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
