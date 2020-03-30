#include "solving/move_inverter.h"
#include "solving/machinery/solve.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"
#ifdef _SE_
#include <se.h>
#endif

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
void move_inverter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  switch (solve_result)
  {
    case previous_move_is_illegal:
      solve_result = immobility_on_next_move;
      break;

    case immobility_on_next_move:
    case previous_move_has_not_solved:
      solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
      break;

    case previous_move_has_solved:
      solve_result = MOVE_HAS_SOLVED_LENGTH();
      break;

    default:
      assert(0);
      solve_result = immobility_on_next_move;
      break;
  }

#ifdef _SE_DECORATE_SOLUTION_
  se_end_set_play();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
