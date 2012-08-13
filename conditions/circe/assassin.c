#include "conditions/circe/assassin.h"
#include "conditions/circe/rebirth_handler.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_assassin(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceAssassinRebirth);
  stip_instrument_moves_no_replay(si,STKingAssassinationAvoider);

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
stip_length_type king_assassination_avoider_attack(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_circe_rebirth_square[nbply]==king_square[White] || current_circe_rebirth_square[nbply]==king_square[Black])
    result = n+2;
  else
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
stip_length_type king_assassination_avoider_defend(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_circe_rebirth_square[nbply]==king_square[White] || current_circe_rebirth_square[nbply]==king_square[Black])
    result = slack_length-1;
  else
    result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static square determine_rebirth_square(Side trait_ply)
{
  square const pi_captured = pprise[nbply];
  square result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  if (pi_captured==vide)
    result = initsquare;
  else
  {
    numecoup const coup_id = current_move[nbply];
    move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
    square const sq_arrival = move_gen_top->arrival;
    square const sq_capture = move_gen_top->capture;
    square const sq_departure = move_gen_top->departure;
    Flags const spec_pi_captured = pprispec[nbply];
    result = rennormal(nbply,
                       pi_captured,
                       spec_pi_captured,
                       sq_capture,
                       sq_departure,
                       sq_arrival,
                       trait_ply);

    if (CondFlag[contactgrid] && nogridcontact(result))
      result = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
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
stip_length_type circe_assassin_rebirth_attack(slice_index si,
                                               stip_length_type n)
{
  square const pi_captured = pprise[nbply];
  Flags const spec_pi_captured = pprispec[nbply];
  square sq_rebirth;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  sq_rebirth = determine_rebirth_square(slices[si].starter);
  if (sq_rebirth==initsquare)
    result = attack(slices[si].next1,n);
  else if (e[sq_rebirth]==vide)
  {
    circe_do_rebirth(sq_rebirth,pi_captured,spec_pi_captured);
    result = attack(slices[si].next1,n);
    circe_undo_rebirth(sq_rebirth);
    current_circe_rebirth_square[nbply] = initsquare;
  }
  else
  {
    piece const assassinated_piece = e[sq_rebirth];
    Flags const assassinated_spec = spec[sq_rebirth];

    --nbpiece[e[sq_rebirth]];

    circe_do_rebirth(sq_rebirth,pi_captured,spec_pi_captured);

    result = attack(slices[si].next1,n);

    current_circe_rebirth_square[nbply] = initsquare;

    --nbpiece[e[sq_rebirth]];
    e[sq_rebirth] = assassinated_piece;
    spec[sq_rebirth] = assassinated_spec;
    ++nbpiece[e[sq_rebirth]];
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
stip_length_type circe_assassin_rebirth_defend(slice_index si,
                                               stip_length_type n)
{
  square const pi_captured = pprise[nbply];
  Flags const spec_pi_captured = pprispec[nbply];
  square sq_rebirth;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  sq_rebirth = determine_rebirth_square(slices[si].starter);
  if (sq_rebirth==initsquare)
    result = defend(slices[si].next1,n);
  else if (e[sq_rebirth]==vide)
  {
    circe_do_rebirth(sq_rebirth,pi_captured,spec_pi_captured);
    result = defend(slices[si].next1,n);
    circe_undo_rebirth(sq_rebirth);
    current_circe_rebirth_square[nbply] = initsquare;
  }
  else
  {
    piece const assassinated_piece = e[sq_rebirth];
    Flags const assassinated_spec = spec[sq_rebirth];

    --nbpiece[e[sq_rebirth]];

    circe_do_rebirth(sq_rebirth,pi_captured,spec_pi_captured);

    result = defend(slices[si].next1,n);

    current_circe_rebirth_square[nbply] = initsquare;

    --nbpiece[e[sq_rebirth]];
    e[sq_rebirth] = assassinated_piece;
    spec[sq_rebirth] = assassinated_spec;
    ++nbpiece[e[sq_rebirth]];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
