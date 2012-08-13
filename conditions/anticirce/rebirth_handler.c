#include "conditions/anticirce/rebirth_handler.h"
#include "pydata.h"
#include "conditions/circe/rebirth_handler.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move_player.h"
#include "solving/post_move_iteration.h"
#include "solving/legal_move_counter.h"
#include "solving/castling.h"
#include "solving/moving_pawn_promotion.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pieces/side_change.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

PieNam current_promotion_of_reborn_moving[maxply+1];

/* Perform an Anticirce rebirth on a specific rebirth square
 * @param sq_rebirth rebirth square
 * @param trait_ply side at the move
 */
void anticirce_do_rebirth_on(square sq_rebirth, Side trait_ply)
{
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_rebirth);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  if (sq_arrival!=sq_rebirth)
  {
    e[sq_rebirth] = e[sq_arrival];
    spec[sq_rebirth] = spec[sq_arrival];

    e[sq_arrival] = vide;
    spec[sq_arrival] = 0;
  }

  current_anticirce_rebirth_square[nbply] = sq_rebirth;

  do_king_rebirth(sq_departure,sq_rebirth);
  restore_castling_rights(sq_rebirth);

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

/* Undo an Anticirce rebirth on a specific rebirth square
 * @param sq_rebirth rebirth square
 * @pre sq_rebirt=!=initsquare
 */
void anticirce_undo_rebirth(square sq_rebirth)
{
  square const sq_arrival= move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_rebirth);
  TraceFunctionParamListEnd();

  assert(sq_rebirth!=initsquare);

  if (sq_rebirth!=sq_arrival)
  {
    --nbpiece[e[sq_rebirth]];
    e[sq_rebirth]= vide;
    spec[sq_rebirth]= 0;
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
      anticirce_do_rebirth_on(sq_rebirth,slices[si].starter);
      result = attack(slices[si].next1,n);
      anticirce_undo_rebirth(sq_rebirth);
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
      anticirce_do_rebirth_on(sq_rebirth,slices[si].starter);
      result = defend(slices[si].next1,n);
      anticirce_undo_rebirth(sq_rebirth);
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

static void place_promotee(void)
{
  square const sq_rebirth = current_anticirce_rebirth_square[nbply];
  piece const pi_reborn = e[sq_rebirth];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --nbpiece[e[sq_rebirth]];
  e[sq_rebirth] = pi_reborn<vide ? -current_promotion_of_reborn_moving[nbply] : current_promotion_of_reborn_moving[nbply];
  ++nbpiece[e[sq_rebirth]];

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
    place_promotee();

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
    place_promotee();

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
