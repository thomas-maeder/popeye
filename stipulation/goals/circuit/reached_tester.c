#include "stipulation/goals/circuit/reached_tester.h"
#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a circuit goal has just been reached
 */

/* Allocate a system of slices that tests whether circuit has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_circuit_reached_tester_system(void)
{
  slice_index result;
  slice_index circuit_tester;
  Goal const goal = { goal_circuit, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  circuit_tester = alloc_pipe(STGoalCircuitReachedTester);
  pipe_link(circuit_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,circuit_tester);

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
stip_length_type goal_circuit_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(current_move[nbply]-1!=nil_coup);

  if (GetPositionInDiagram(spec[sq_arrival])==sq_arrival)
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
