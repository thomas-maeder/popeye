#include "solving/post_move_iteration.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type post_move_iteration_initialiser_attack(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(slices[si].next1,n);

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

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type post_move_iteration_initialiser_defend(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(slices[si].next1,n);

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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type post_move_iteration_avoider_attack(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  post_move_iteration_locked[nbply] = true;
  TraceValue("%u",nbply);TraceValue("%u\n",post_move_iteration_locked[nbply]);

  result = attack(slices[si].next1,n);

  post_move_iteration_locked[nbply] = false;
  TraceValue("%u",nbply);  TraceValue("%u\n",post_move_iteration_locked[nbply]);

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
  stip_instrument_moves_replay(si,STPostMoveIterationAvoider);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
