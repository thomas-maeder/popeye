#include "solving/goals/prerequisite_optimiser.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/machinery/slack_length.h"
#include "solving/goals/prerequisite_guards.h"
#include "solving/avoid_unsolvable.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STPrerequisiteOptimiser defender slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_prerequisite_optimiser_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPrerequisiteOptimiser);

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
void goal_prerequisite_optimiser_solve(slice_index si)
{
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(max_unsolvable<=slack_length);

  if (goal_preprequisites_met[nbply]==0)
  {
    max_unsolvable = slack_length+1;
    TraceValue("->%u\n",max_unsolvable);
  }

  pipe_solve_delegate(si);

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u\n",max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
