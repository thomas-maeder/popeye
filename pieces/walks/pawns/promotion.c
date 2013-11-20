#include "pieces/walks/pawns/promotion.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>

enum
{
  stack_size = max_nr_promotions_per_ply*maxply+1
};

static pieces_pawns_promotion_sequence_type promotion_stack[stack_size];
static unsigned int stack_pointer;

static post_move_iteration_id_type prev_post_move_iteration_id[stack_size];

move_effect_journal_index_type promotion_horizon;

/* Find the last square occupied by a piece since we last checked.
 * @param base index of move effects that have already been dealt with
 * @return the square; initsquare if there isn't any
 */
square find_potential_promotion_square(move_effect_journal_index_type base)
{
  move_effect_journal_index_type curr = move_effect_journal_base[nbply+1];

  while (curr>base)
  {
    --curr;

    switch (move_effect_journal[curr].type)
    {
      case move_effect_piece_movement:
        return move_effect_journal[curr].u.piece_movement.to;

      case move_effect_piece_readdition:
        return move_effect_journal[curr].u.piece_addition.on;

      default:
        break;
    }
  }

  return initsquare;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type pawn_promoter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const save_horizon = promotion_horizon;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const sq_potential_promotion = find_potential_promotion_square(promotion_horizon);

    assert(stack_pointer<stack_size);

    promotion_horizon = move_effect_journal_base[nbply+1];

    if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[stack_pointer])
      pieces_pawns_start_promotee_sequence(sq_potential_promotion,
                                           &promotion_stack[stack_pointer]);

    if (promotion_stack[stack_pointer].promotee==Empty)
    {
      ++stack_pointer;
      result = solve(slices[si].next1,n);
      --stack_pointer;
    }
    else
    {
      move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                          sq_potential_promotion,
                                          promotion_stack[stack_pointer].promotee);

      ++stack_pointer;
      result = solve(slices[si].next1,n);
      --stack_pointer;

      if (!post_move_iteration_locked[nbply])
      {
        pieces_pawns_continue_promotee_sequence(&promotion_stack[stack_pointer]);
        if (promotion_stack[stack_pointer].promotee!=Empty)
          lock_post_move_iterations();
      }

      prev_post_move_iteration_id[stack_pointer] = post_move_iteration_id[nbply];
    }

    promotion_horizon = save_horizon;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery with the promotion of the moving pawn
 * @param si identifies the root slice of the solving machinery
 */
void pieces_pawns_promotion_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STBeforePawnPromotion);
  stip_instrument_moves(si,STPawnPromoter);
  stip_instrument_moves(si,STLandingAfterPawnPromotion);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
