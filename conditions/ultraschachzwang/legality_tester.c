#include "conditions/ultraschachzwang/legality_tester.h"
#include "stipulation/pipe.h"
#include "solving/check.h"
#include "solving/ply.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

/* Allocate a STUltraschachzwangLegalityTester slice.
 * @return index of allocated slice
 */
slice_index alloc_ultraschachzwang_legality_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STUltraschachzwangLegalityTester);

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
void ultraschachzwang_legality_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,!is_in_check(advers(trait[nbply])));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
