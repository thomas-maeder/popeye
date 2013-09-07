#include "conditions/anticirce/promotion.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/circe/circe.h"
#include "pieces/walks/pawns/promotion.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

pieces_pawns_promotion_sequence_type current_promotion_of_reborn_moving[maxply+1];

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
stip_length_type anticirce_reborn_promoter_solve(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const rebirth = anticirce_find_current_rebirth();
    if (rebirth==move_effect_journal_base[nbply+1])
      result = solve(slices[si].next1,n);
    else
    {
      square const sq_rebirth = move_effect_journal[rebirth].u.piece_addition.on;

      if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
        pieces_pawns_initialise_promotion_sequence(sq_rebirth,
                                                   &current_promotion_of_reborn_moving[nbply]);

      if (current_promotion_of_reborn_moving[nbply].promotee==Empty)
        result = solve(slices[si].next1,n);
      else
      {
        move_effect_journal_do_piece_change(move_effect_reason_promotion_of_reborn,
                                            sq_rebirth,
                                            current_promotion_of_reborn_moving[nbply].promotee);

        result = solve(slices[si].next1,n);

        if (!post_move_iteration_locked[nbply])
        {
          pieces_pawns_continue_promotion_sequence(&current_promotion_of_reborn_moving[nbply]);
          TracePiece(current_promotion_of_reborn_moving[nbply].promotee);TraceText("\n");
          if (current_promotion_of_reborn_moving[nbply].promotee!=Empty)
            lock_post_move_iterations();
        }

        prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_promotion(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticirceRebornPromoter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
