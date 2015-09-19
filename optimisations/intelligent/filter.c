#include "optimisations/intelligent/filter.h"
#include "stipulation/stipulation.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static boolean Intelligent(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solutions_found = false;
  intelligent_duplicate_avoider_init();
  IntelligentRegulargoal_types(si);
  intelligent_duplicate_avoider_cleanup();
  result = solutions_found;

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
void intelligent_filter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_solves_exactly_if(Intelligent(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
