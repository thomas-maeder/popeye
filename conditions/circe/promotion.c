#include "conditions/circe/promotion.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "conditions/circe/circe.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

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
stip_length_type circe_promoter_solve(slice_index si, stip_length_type n)
{
  square const sq_rebirth = current_circe_rebirth_square[nbply];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    /* check for both sides - this has to work for Circe Parrain and neutrals as well! */
    if ((TSTFLAG(spec[sq_rebirth],White) && has_pawn_reached_promotion_square(White,sq_rebirth))
        || (TSTFLAG(spec[sq_rebirth],Black) && has_pawn_reached_promotion_square(Black,sq_rebirth)))
      current_promotion_of_capturee[nbply] = promotee_chain[promotee_chain_orthodox][Empty];
    else
      current_promotion_of_capturee[nbply] = Empty;
  }

  if (current_promotion_of_capturee[nbply]==Empty)
    result = solve(slices[si].next1,n);
  else
  {
    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        sq_rebirth,
                                        e[sq_rebirth]<vide
                                        ? -current_promotion_of_capturee[nbply]
                                        : current_promotion_of_capturee[nbply]);

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_capturee[nbply] = promotee_chain[promotee_chain_orthodox][current_promotion_of_capturee[nbply]];
      TracePiece(current_promotion_of_capturee[nbply]);TraceText("\n");

      if (current_promotion_of_capturee[nbply]!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STCirceRebirthPromoter);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_promoters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STCirceDetermineRebirthSquare,
                                           &instrument_rebirth);
  stip_structure_traversal_override_single(&st,
                                           STSuperCirceRebirthHandler,
                                           &instrument_rebirth);
  stip_structure_traversal_override_single(&st,
                                           STCirceParrainDetermineRebirth,
                                           &instrument_rebirth);
  stip_structure_traversal_override_single(&st,
                                           STTakeMakeCirceDetermineRebirthSquares,
                                           &instrument_rebirth);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
