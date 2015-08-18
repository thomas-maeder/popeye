#include "solving/fork_on_remaining.h"
#include "stipulation/stipulation.h"
#include "solving/machinery/slack_length.h"
#include "solving/binary.h"
#include "stipulation/binary.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STForkOnRemaining slice.
 * @param op1 identifies direction taken if threshold is not met
 * @param op2 identifies direction taken if threshold is met
 * @param threshold at which move should we continue with op2?
 * @return index of allocated slice
 */
slice_index alloc_fork_on_remaining_slice(slice_index op1,
                                          slice_index op2,
                                          stip_length_type threshold)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParam("%u",threshold);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STForkOnRemaining,op1,op2);
  SLICE_U(result).fork_on_remaining.threshold = threshold;

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
void fork_on_remaining_solve(slice_index si)
{
  stip_length_type const threshold = SLICE_U(si).fork_on_remaining.threshold;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  binary_solve_if_then_else(si,solve_nr_remaining<=slack_length+threshold);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
