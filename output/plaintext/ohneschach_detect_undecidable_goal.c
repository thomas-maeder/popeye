#include "output/plaintext/ohneschach_detect_undecidable_goal.h"
#include "stipulation/pipe.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "conditions/ohneschach.h"
#include "output/plaintext/message.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

/* Allocate a STOhneschachDetectUndecidableGoal slice.
 * @return index of allocated slice
 */
slice_index alloc_ohneschach_detect_undecidable_goal_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOhneschachDetectUndecidableGoal);

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
void ohneschach_detect_undecidable_goal_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",ohneschach_undecidable_goal_detected[nbply+1]);
  TraceEOL();
  if (ohneschach_undecidable_goal_detected[nbply+1])
  {
    ohneschach_undecidable_goal_detected[nbply+1] = false;
    solve_result = previous_move_is_illegal;
    output_plaintext_message(ChecklessUndecidable);
    output_plaintext_message(NewLine);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
