#include "solving/post_move_iteration.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

boolean post_move_iteration_locked[maxply+1];
unsigned int post_move_iteration_id[maxply+1];

/* Lock post move iterations in the current move retraction
 */
void lock_post_move_iterations(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  post_move_iteration_locked[nbply] = true;
  ++post_move_iteration_id[nbply];
  TraceValue("%u",nbply);TraceValue("%u\n",post_move_iteration_id[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type post_move_iteration_initialiser_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(slices[si].next1,n);

  if (post_move_iteration_locked[nbply])
    post_move_iteration_locked[nbply] = false;
  else
  {
    --current_move[nbply];
    ++post_move_iteration_id[nbply];
    TraceValue("%u",nbply);TraceValue("%u\n",post_move_iteration_id[nbply]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with post move iteration slices
 */
void stip_insert_post_move_iteration(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STPostMoveIterationInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
