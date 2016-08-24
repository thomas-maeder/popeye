#include "solving/goals/king_capture.h"
#include "solving/pipe.h"
#include "position/position.h"
#include "debugging/trace.h"

/* This module provides functionality dealing with slices that detect
 * whether an any goal has just been reached
 */

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void goal_king_capture_reached_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,SLICE_STARTER(si));
  TraceEOL();
  pipe_this_move_doesnt_solve_if(si,being_solved.king_square[SLICE_STARTER(si)]!=initsquare);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
