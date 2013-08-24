#include "conditions/circe/promotion.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "conditions/circe/circe.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

pieces_pawns_promotion_sequence_type reborn_pawn_promotion_state[maxply+1];

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
    pieces_pawns_initialise_promotion_sequence(sq_rebirth,
                                               &reborn_pawn_promotion_state[nbply]);

  if (reborn_pawn_promotion_state[nbply].promotee==Empty)
    result = solve(slices[si].next1,n);
  else
  {
    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        sq_rebirth,
                                        reborn_pawn_promotion_state[nbply].promotee);

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      pieces_pawns_continue_promotion_sequence(&reborn_pawn_promotion_state[nbply]);
      if (reborn_pawn_promotion_state[nbply].promotee!=Empty)
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
