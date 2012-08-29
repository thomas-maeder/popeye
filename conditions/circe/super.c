#include "conditions/circe/super.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/rebirth_handler.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static post_move_iteration_id_type prev_post_move_iteration_id_rebirth[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_no_rebirth[maxply+1];

static boolean is_rebirth_square_dirty[maxply+1];

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type supercirce_no_rebirth_fork_attack(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_no_rebirth[nbply])
  {
    result = attack(slices[si].next2,n);
    if (!post_move_iteration_locked[nbply])
      lock_post_move_iterations();
  }
  else
    result = attack(slices[si].next1,n);

  prev_post_move_iteration_id_no_rebirth[nbply] = post_move_iteration_id[nbply];

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
stip_length_type supercirce_no_rebirth_fork_defend(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_no_rebirth[nbply])
  {
    result = defend(slices[si].next2,n);
    if (!post_move_iteration_locked[nbply])
      lock_post_move_iterations();
  }
  else
    result = defend(slices[si].next1,n);

  prev_post_move_iteration_id_no_rebirth[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean advance_rebirth_square(void)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do
  {
    if (current_circe_rebirth_square[nbply]<square_h8)
      ++current_circe_rebirth_square[nbply];
    else
    {
      result = false;
      break;
    }
  } while (e[current_circe_rebirth_square[nbply]]!=vide);

  is_rebirth_square_dirty[nbply] = false;

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
stip_length_type supercirce_rebirth_handler_attack(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_rebirth[nbply])
  {
    current_circe_rebirth_square[nbply] = square_a1-1;
    is_rebirth_square_dirty[nbply] = true;
  }

  if (is_rebirth_square_dirty[nbply] && !advance_rebirth_square())
  {
    current_circe_rebirth_square[nbply] = initsquare;
    result = n+2;
  }
  else
  {
    square const sq_rebirth = current_circe_rebirth_square[nbply];

    circe_do_rebirth(sq_rebirth,pprise[nbply],pprispec[nbply]);
    result = attack(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      is_rebirth_square_dirty[nbply] = true;
      lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_rebirth[nbply] = post_move_iteration_id[nbply];

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
stip_length_type supercirce_rebirth_handler_defend(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_rebirth[nbply])
  {
    current_circe_rebirth_square[nbply] = square_a1-1;
    is_rebirth_square_dirty[nbply] = true;
  }

  if (is_rebirth_square_dirty[nbply] && !advance_rebirth_square())
  {
    current_circe_rebirth_square[nbply] = initsquare;
    result = n+2;
  }
  else
  {
    square const sq_rebirth = current_circe_rebirth_square[nbply];

    circe_do_rebirth(sq_rebirth,pprise[nbply],pprispec[nbply]);
    result = defend(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      is_rebirth_square_dirty[nbply] = true;
      lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_rebirth[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_supercirce_rebirth_handlers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_insert_rebirth_avoider(si,STSuperCirceNoRebirthFork);
  stip_instrument_moves_no_replay(si,STSuperCirceRebirthHandler);
  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
