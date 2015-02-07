#include "solving/testing_pipe.h"
#include "solving/has_solution_type.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Solve the next2 part of a fork
 * @param si identifies the fork slice
 * @param n maximum number of moves (typically slack_length or
 *         length_unspecified)
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type testing_pipe_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type const save_solve_nr_remaining = solve_nr_remaining;
  stip_length_type const save_solve_result = solve_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  solve_nr_remaining = n;

  solve(SLICE_NEXT2(si));
  result = solve_result;

  solve_nr_remaining = save_solve_nr_remaining;
  solve_result = save_solve_result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
