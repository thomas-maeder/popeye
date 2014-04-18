#include "solving/goals/circuit_by_rebirth.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether a circuit (by rebirth) goal has just been reached
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
void goal_circuit_by_rebirth_reached_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(CURRMOVE_OF_PLY(nbply)!=nil_coup);

  /* goal is only reachable in some fairy conditions */
  solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
