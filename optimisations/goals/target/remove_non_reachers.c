#include "optimisations/goals/target/remove_non_reachers.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STTargetRemoveNonReachers slice.
 * @param target target square to be reached
 * @return index of allocated slice
 */
slice_index alloc_target_remove_non_reachers_slice(square target)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STTargetRemoveNonReachers);
  slices[result].u.goal_handler.goal.target = target;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static square target_square = initsquare;

static boolean reaches_targetsquare(square sq_departure,
                                    square sq_arrival,
                                    square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = sq_arrival==target_square;

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
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type target_remove_non_reachers_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(target_square==initsquare);

  target_square = slices[si].u.goal_handler.goal.target;
  move_generator_filter_moves(&reaches_targetsquare);

  target_square = initsquare;

  result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
