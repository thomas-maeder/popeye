#include "solving/conditional_pipe.h"
#include "solving/has_solution_type.h"
#include "solving/fork.h"
#include "debugging/trace.h"

/* Solve the next2 part of a conditional pipe
 * @param si identifies the fork slice
 * @return one of
 *        previous_move_has_solved
 *        previous_move_is_illegal
 *        next_move_has_no_solution
 *        next_move_has_solution
 *        immobility_on_next_move
 */
stip_length_type conditional_pipe_solve(slice_index si)
{
  stip_length_type result;
  stip_length_type const save_solve_nr_remaining = solve_nr_remaining;
  stip_length_type const save_solve_result = solve_result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_nr_remaining = length_unspecified;

  fork_solve_delegate(si);
  result = solve_result;

  solve_nr_remaining = save_solve_nr_remaining;
  solve_result = save_solve_result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
