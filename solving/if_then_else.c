#include "solving/if_then_else.h"
#include "solving/has_solution_type.h"
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
void if_then_else_solve(slice_index si)
{
  stip_length_type const save_solve_nr_remaining = MOVE_HAS_SOLVED_LENGTH();
  slice_index const op1 = SLICE_NEXT1(si);
  slice_index const op2 = SLICE_NEXT2(si);
  slice_index const condition = SLICE_U(si).if_then_else.condition;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_nr_remaining = previous_move_has_solved;
  solve(condition);
  solve_nr_remaining = save_solve_nr_remaining;

  solve(solve_result==previous_move_has_solved ? op2 : op1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
