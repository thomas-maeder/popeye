#include "conditions/anticirce/rebirth_handler.h"
#include "pydata.h"
#include "conditions/circe/rebirth_handler.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/post_move_iteration.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

PieNam current_promotion_of_reborn_moving[maxply+1];

/* Perform an Anticirce rebirth
 */
void anticirce_do_rebirth(move_effect_reason_type reason)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  piece const reborn = e[sq_arrival];
  piece const rebornspec = spec[sq_arrival];
  square const sq_rebirth = current_anticirce_rebirth_square[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_removal(reason,sq_arrival);
  move_effect_journal_do_piece_addition(reason,sq_rebirth,reborn,rebornspec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void determine_rebirth_square(Side trait_ply)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  if (pprise[nbply]==vide)
    current_anticirce_rebirth_square[nbply] = initsquare;
  else
  {
    square const sq_departure = move_generation_stack[current_move[nbply]].departure;
    square const sq_capture = move_generation_stack[current_move[nbply]].capture;
    square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
    piece const pi_arriving = e[sq_arrival];
    Flags const spec_pi_moving = spec[sq_arrival];

    current_anticirce_rebirth_square[nbply] = (*antirenai)(nbply,
                                                           pi_arriving,
                                                           spec_pi_moving,
                                                           sq_capture,
                                                           sq_departure,
                                                           sq_arrival,
                                                           advers(trait_ply));
  }

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
stip_length_type anticirce_rebirth_handler_attack(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  determine_rebirth_square(slices[si].starter);

  {
    square const sq_rebirth = current_anticirce_rebirth_square[nbply];
    if (sq_rebirth==initsquare)
      result = attack(slices[si].next1,n);
    else if ((!AntiCirCheylan
              && sq_rebirth==move_generation_stack[current_move[nbply]].arrival)
             || e[sq_rebirth]==vide)
    {
      anticirce_do_rebirth(move_effect_reason_anticirce_rebirth);
      result = attack(slices[si].next1,n);
    }
    else
      result = n+2;
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
stip_length_type anticirce_rebirth_handler_defend(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  determine_rebirth_square(slices[si].starter);

  {
    square const sq_rebirth = current_anticirce_rebirth_square[nbply];
    if (sq_rebirth==initsquare)
      result = defend(slices[si].next1,n);
    else if ((!AntiCirCheylan
              && sq_rebirth==move_generation_stack[current_move[nbply]].arrival)
             || e[sq_rebirth]==vide)
    {
      anticirce_do_rebirth(move_effect_reason_anticirce_rebirth);
      result = defend(slices[si].next1,n);
    }
    else
      result = slack_length-1;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void init_promotee(Side trait_ply)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  if (has_pawn_reached_promotion_square(trait_ply,current_anticirce_rebirth_square[nbply]))
    current_promotion_of_reborn_moving[nbply] = getprompiece[Empty];
  else
    current_promotion_of_reborn_moving[nbply] = Empty;

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
stip_length_type anticirce_reborn_promoter_attack(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    init_promotee(slices[si].starter);

  if (current_promotion_of_reborn_moving[nbply]==Empty)
    result = attack(slices[si].next1,n);
  else
  {
    square const sq_rebirth = current_anticirce_rebirth_square[nbply];
    piece const pi_reborn = e[sq_rebirth];
    piece const promotee = pi_reborn<vide ? -current_promotion_of_reborn_moving[nbply] : current_promotion_of_reborn_moving[nbply];

    move_effect_journal_do_piece_change(move_effect_reason_promotion_of_reborn,
                                        sq_rebirth,
                                        promotee);

    result = attack(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_reborn_moving[nbply] = getprompiece[current_promotion_of_reborn_moving[nbply]];
      TracePiece(current_promotion_of_reborn_moving[nbply]);TraceText("\n");
      if (current_promotion_of_reborn_moving[nbply]!=Empty)
        lock_post_move_iterations();
    }

    prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
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
stip_length_type anticirce_reborn_promoter_defend(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    init_promotee(slices[si].starter);

  if (current_promotion_of_reborn_moving[nbply]==Empty)
    result = defend(slices[si].next1,n);
  else
  {
    square const sq_rebirth = current_anticirce_rebirth_square[nbply];
    piece const pi_reborn = e[sq_rebirth];
    piece const promotee = pi_reborn<vide ? -current_promotion_of_reborn_moving[nbply] : current_promotion_of_reborn_moving[nbply];

    move_effect_journal_do_piece_change(move_effect_reason_promotion_of_reborn,
                                        sq_rebirth,
                                        promotee);

    result = defend(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_reborn_moving[nbply] = getprompiece[current_promotion_of_reborn_moving[nbply]];
      if (current_promotion_of_reborn_moving[nbply]!=Empty)
        lock_post_move_iterations();
    }

    prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_rebirth_handlers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticirceRebirthHandler);

  if (anyanticirprom)
    stip_instrument_moves(si,STAnticirceRebornPromoter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
