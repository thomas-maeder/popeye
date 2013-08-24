#include "solving/moving_pawn_promotion.h"
#include "pieces/walks/pawns/promotion.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

pieces_pawns_promotion_sequence_type moving_pawn_promotion_state[maxply+1];

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
stip_length_type moving_pawn_promoter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
      pieces_pawns_initialise_promotion_sequence(pos,
                                                 &moving_pawn_promotion_state[nbply]);

    if (moving_pawn_promotion_state[nbply].promotee==Empty)
      result = solve(slices[si].next1,n);
    else
    {
      move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                          pos,
                                          moving_pawn_promotion_state[nbply].promotee);

      result = solve(slices[si].next1,n);

      if (!post_move_iteration_locked[nbply])
      {
        pieces_pawns_continue_promotion_sequence(&moving_pawn_promotion_state[nbply]);
        if (moving_pawn_promotion_state[nbply].promotee!=Empty)
          lock_post_move_iterations();
      }
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_moving_pawn_promoters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMovingPawnPromoter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
