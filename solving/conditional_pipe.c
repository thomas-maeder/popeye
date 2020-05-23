#include "solving/conditional_pipe.h"
#include "solving/has_solution_type.h"
#include "solving/fork.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Solve the next2 part of a conditional pipe
 * @param si identifies the fork slice
 * @return one of
 *        previous_move_has_solved
 *        previous_move_has_not_solved
 *        previous_move_is_illegal
 *        immobility_on_next_move
 */
conditional_pipe_solve_return_type conditional_pipe_solve_delegate(slice_index si)
{
  conditional_pipe_solve_return_type result;
  stip_length_type const save_solve_nr_remaining = solve_nr_remaining;
  stip_length_type const save_solve_result_min = solve_result_min();
  stip_length_type const save_solve_result_max = solve_result_max();
  
  {
    enum {
      ensure_previous_move_is_illegal_plus_1_equals_immobility_on_next_move    = 1/((previous_move_is_illegal+1)==immobility_on_next_move),
      ensure_immobility_on_next_move_plus_1_equals_previous_move_has_solved    = 1/((immobility_on_next_move+1)==previous_move_has_solved),
      ensure_previous_move_has_solved_plus_1_equals_next_move_has_solution     = 1/((previous_move_has_solved+1)==next_move_has_solution),
      ensure_next_move_has_solution_plus_1_equals_previous_move_has_not_solved = 1/((next_move_has_solution+1)==previous_move_has_not_solved)
    };
  }

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_nr_remaining = length_unspecified;

  fork_solve_delegate(si);
  result.result_min = solve_result_min();
  if (result.result_min < previous_move_is_illegal)
    result.result_min = previous_move_is_illegal;
  else if (result.result_min == next_move_has_solution)
    ++result.result_min;
  result.result_max = solve_result_max();
  if (result.result_max > previous_move_has_not_solved)
    result.result_max = previous_move_has_not_solved;
  else if (result.result_max == next_move_has_solution)
    --result.result_max;
  assert(result.result_min <= result.result_max);

  solve_nr_remaining = save_solve_nr_remaining;
  set_solve_result_range(save_solve_result_min, save_solve_result_max);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result.result_min);
  TraceFunctionResult("%u",result.result_max);
  TraceFunctionResultEnd();
  return result;
}
