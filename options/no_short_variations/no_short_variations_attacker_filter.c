#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "solving/has_solution_type.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/battle_play/branch.h"
#include "solving/machinery/solve.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/testing_pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STNoShortVariations slice.
 * @return index of allocated slice
 */
slice_index alloc_no_short_variations_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_testing_pipe(STNoShortVariations);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a short solution after the defense played
 * in a slice
 * @param si identifies slice that just played the defense
 * @return true iff there is a short solution
 */
static boolean has_short_solution(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = testing_pipe_solve(si,solve_nr_remaining-2)<=solve_nr_remaining-2;

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
void no_short_variations_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (solve_nr_remaining>next_move_has_solution
      && encore() /* otherwise we are solving threats */
      && has_short_solution(si))
    solve_result = MOVE_HAS_SOLVED_LENGTH();
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
