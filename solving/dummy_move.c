#include "solving/dummy_move.h"
#include "solving/machinery/solve.h"
#include "solving/ply.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "position/effects/null_move.h"
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
void dummy_move_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=next_move_has_solution);

  /* insert an empty ply for the dummy defense */
  nextply(advers(trait[nbply]));
  move_effect_journal_do_null_move();
  pipe_solve_delegate(si);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
