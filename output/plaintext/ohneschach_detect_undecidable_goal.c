#include "output/plaintext/ohneschach_detect_undecidable_goal.h"
#include "stipulation/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "conditions/ohneschach.h"
#include "debugging/trace.h"
#include "pyproc.h"
#include "pymsg.h"

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type ohneschach_detect_undecidable_goal_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u\n",ohneschach_undecidable_goal_detected[nbply+1]);
  if (ohneschach_undecidable_goal_detected[nbply+1])
  {
    ohneschach_undecidable_goal_detected[nbply+1] = false;
    result = previous_move_is_illegal;
    StdChar(' ');
    Message(ChecklessUndecidable);
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
