#include "conditions/circe/super.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/circe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static post_move_iteration_id_type prev_post_move_iteration_id_rebirth[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_no_rebirth[maxply+1];

static boolean is_rebirth_square_dirty[maxply+1];

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
stip_length_type supercirce_no_rebirth_fork_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_no_rebirth[nbply])
  {
    result = solve(slices[si].next2,n);
    if (!post_move_iteration_locked[nbply])
      lock_post_move_iterations();
  }
  else
    result = solve(slices[si].next1,n);

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
      current_circe_rebirth_square[nbply] = initsquare;
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
stip_length_type supercirce_rebirth_handler_solve(slice_index si,
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
    result = previous_move_is_illegal;
  else
  {
    result = solve(slices[si].next1,n);

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
  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  stip_instrument_moves(si,STSuperCirceRebirthHandler);
  stip_instrument_moves(si,STCircePlaceReborn);
  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
