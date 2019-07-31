#include "solving/goals/reached_tester.h"
#include "solving/has_solution_type.h"
#include "solving/conditional_pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
void goal_reached_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (conditional_pipe_solve_delegate(si))
  {
    case previous_move_is_illegal:
      solve_result = previous_move_is_illegal;
      break;

    case previous_move_has_not_solved:
    case immobility_on_next_move:
      solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
      break;

    case previous_move_has_solved:
      pipe_solve_delegate(si);
      break;

    default:
      assert(0);
      solve_result = previous_move_is_illegal;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
