#include "solving/find_move.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "solving/post_move_iteration.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STFindAttack slice.
 * @return index of allocated slice
 */
slice_index alloc_find_attack_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STFindAttack);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
void find_attack_solve(slice_index si)
{
  stip_length_type result_intermediate = MOVE_HAS_NOT_SOLVED_LENGTH();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  while (encore() && result_intermediate>MOVE_HAS_SOLVED_LENGTH())
  {
    pipe_solve_delegate(si);
    assert(slack_length<=solve_result || solve_result==this_move_is_illegal);
    if (slack_length<solve_result && solve_result<result_intermediate)
      result_intermediate = solve_result;
  }

  solve_result = result_intermediate;

  if (encore())
    post_move_iteration_cancel();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STFindDefense slice.
 * @return index of allocated slice
 */
slice_index alloc_find_defense_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STFindDefense);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
void find_defense_solve(slice_index si)
{
  stip_length_type result_intermediate = immobility_on_next_move;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  while (result_intermediate<=MOVE_HAS_SOLVED_LENGTH() && encore())
  {
    pipe_solve_delegate(si);
    assert(slack_length<=solve_result || solve_result==this_move_is_illegal);
    if (result_intermediate<solve_result)
      result_intermediate = solve_result;
  }

  solve_result = result_intermediate;

  if (encore())
    post_move_iteration_cancel();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
