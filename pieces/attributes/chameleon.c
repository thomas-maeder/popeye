#include "pieces/attributes/chameleon.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move_player.h"
#include "solving/post_move_iteration.h"
#include "solving/moving_pawn_promotion.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean promotion_of_moving_into_chameleon[maxply+1];
boolean promotion_of_circe_reborn_into_chameleon[maxply+1];
static boolean promotion_of_anticirce_reborn_into_chameleon[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id_moving[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_circe_reborn[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_anticirce_reborn[maxply+1];

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type chameleon_promote_moving_into_attack(slice_index si,
                                                      stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_moving[nbply])
    promotion_of_moving_into_chameleon[nbply] = has_pawn_reached_promotion_square(slices[si].starter,sq_arrival);

  if (promotion_of_moving_into_chameleon[nbply])
  {
    SETFLAG(spec[move_generation_stack[current_move[nbply]].arrival],Chameleon);

    result = attack(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_moving_into_chameleon[nbply] = false;
      TraceValue("%u\n",promotion_of_moving_into_chameleon[nbply]);
      lock_post_move_iterations();
    }
  }
  else
    result = attack(slices[si].next1,n);

  prev_post_move_iteration_id_moving[nbply] = post_move_iteration_id[nbply];

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
stip_length_type chameleon_promote_moving_into_defend(slice_index si,
                                                      stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_moving[nbply])
    promotion_of_moving_into_chameleon[nbply] = has_pawn_reached_promotion_square(slices[si].starter,sq_arrival);

  if (promotion_of_moving_into_chameleon[nbply])
  {
    SETFLAG(spec[move_generation_stack[current_move[nbply]].arrival],Chameleon);

    result = defend(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_moving_into_chameleon[nbply] = false;
      lock_post_move_iterations();
    }
  }
  else
    result = defend(slices[si].next1,n);

  prev_post_move_iteration_id_moving[nbply] = post_move_iteration_id[nbply];

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
stip_length_type chameleon_promote_circe_reborn_into_attack(slice_index si,
                                                            stip_length_type n)
{
  square const sq_rebirth = current_circe_rebirth_square[nbply];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_circe_reborn[nbply])
    promotion_of_circe_reborn_into_chameleon[nbply] = ((TSTFLAG(spec[sq_rebirth],White) && has_pawn_reached_promotion_square(White,sq_rebirth))
                                                   || (TSTFLAG(spec[sq_rebirth],Black) && has_pawn_reached_promotion_square(Black,sq_rebirth)));

  if (promotion_of_circe_reborn_into_chameleon[nbply])
  {
    SETFLAG(spec[sq_rebirth],Chameleon);

    result = attack(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_circe_reborn_into_chameleon[nbply] = false;
      lock_post_move_iterations();
    }
  }
  else
    result = attack(slices[si].next1,n);

  prev_post_move_iteration_id_circe_reborn[nbply] = post_move_iteration_id[nbply];

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
stip_length_type chameleon_promote_circe_reborn_into_defend(slice_index si,
                                                            stip_length_type n)
{
  square const sq_rebirth = current_circe_rebirth_square[nbply];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_circe_reborn[nbply])
    promotion_of_circe_reborn_into_chameleon[nbply] = ((TSTFLAG(spec[sq_rebirth],White) && has_pawn_reached_promotion_square(White,sq_rebirth))
                                                   || (TSTFLAG(spec[sq_rebirth],Black) && has_pawn_reached_promotion_square(Black,sq_rebirth)));

  if (promotion_of_circe_reborn_into_chameleon[nbply])
  {
    SETFLAG(spec[sq_rebirth],Chameleon);

    result = defend(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_circe_reborn_into_chameleon[nbply] = false;
      lock_post_move_iterations();
    }
  }
  else
    result = defend(slices[si].next1,n);

  prev_post_move_iteration_id_circe_reborn[nbply] = post_move_iteration_id[nbply];

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
stip_length_type chameleon_promote_anticirce_reborn_into_attack(slice_index si,
                                                                stip_length_type n)
{
  square const sq_rebirth = current_anticirce_rebirth_square[nbply];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_anticirce_reborn[nbply])
    promotion_of_anticirce_reborn_into_chameleon[nbply] = ((TSTFLAG(spec[sq_rebirth],White) && has_pawn_reached_promotion_square(White,sq_rebirth))
                                                           || (TSTFLAG(spec[sq_rebirth],Black) && has_pawn_reached_promotion_square(Black,sq_rebirth)));

  if (promotion_of_anticirce_reborn_into_chameleon[nbply])
  {
    SETFLAG(spec[sq_rebirth],Chameleon);

    result = attack(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_anticirce_reborn_into_chameleon[nbply] = false;
      lock_post_move_iterations();
    }
  }
  else
    result = attack(slices[si].next1,n);

  prev_post_move_iteration_id_anticirce_reborn[nbply] = post_move_iteration_id[nbply];

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
stip_length_type chameleon_promote_anticirce_reborn_into_defend(slice_index si,
                                                                stip_length_type n)
{
  square const sq_rebirth = current_anticirce_rebirth_square[nbply];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_anticirce_reborn[nbply])
    promotion_of_anticirce_reborn_into_chameleon[nbply] = ((TSTFLAG(spec[sq_rebirth],White) && has_pawn_reached_promotion_square(White,sq_rebirth))
                                                           || (TSTFLAG(spec[sq_rebirth],Black) && has_pawn_reached_promotion_square(Black,sq_rebirth)));

  if (promotion_of_anticirce_reborn_into_chameleon[nbply])
  {
    SETFLAG(spec[sq_rebirth],Chameleon);

    result = defend(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_anticirce_reborn_into_chameleon[nbply] = false;
      lock_post_move_iterations();
    }
  }
  else
    result = defend(slices[si].next1,n);

  prev_post_move_iteration_id_anticirce_reborn[nbply] = post_move_iteration_id[nbply];

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
stip_length_type chameleon_arriving_adjuster_attack(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[sq_arrival],Chameleon))
    replace_arriving_piece(champiece(e[sq_arrival]));
  result = attack(slices[si].next1,n);

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
stip_length_type chameleon_arriving_adjuster_defend(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[sq_arrival],Chameleon))
    replace_arriving_piece(champiece(e[sq_arrival]));
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_chameleon(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STChameleonArrivingAdjuster);
  stip_instrument_moves(si,STPromoteMovingIntoChameleon);

  if (anycirprom)
    stip_instrument_moves(si,STPromoteCirceRebornIntoChameleon);

  if (anyanticirprom)
    stip_instrument_moves(si,STPromoteAnticirceRebornIntoChameleon);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
