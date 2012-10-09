#include "stipulation/goals/enpassant/reached_tester.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a enpassant goal has just been reached
 */

/* Allocate a system of slices that tests whether enpassant has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_enpassant_reached_tester_system(void)
{
  slice_index result;
  slice_index enpassant_tester;
  Goal const goal = { goal_ep, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  enpassant_tester = alloc_pipe(STGoalEnpassantReachedTester);
  pipe_link(enpassant_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,enpassant_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goal_enpassant_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbply!=nil_ply);

  if (sq_capture!=move_generation_stack[current_move[nbply]].arrival
      && sq_capture<=square_h8
      && is_pawn(pjoue[nbply]))
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
