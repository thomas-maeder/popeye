#include "solving/recursion_stopper.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "solving/ply.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

boolean recursion_stopped;

/* Allocate a STRecursionStopper slice
 * @return newly allocated slice
 */
slice_index alloc_recursion_stopper_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRecursionStopper);

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
void recursion_stopper_solve(slice_index si)
{
#if defined(DOTRACE)
  /* empirically determined at 1 workstation */
  ply const stop_at_ply = 200;
#else
  ply const stop_at_ply = maxply-3;
#endif

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (nbply>stop_at_ply)
  {
    recursion_stopped = true;
    solve_result = previous_move_is_illegal;
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
