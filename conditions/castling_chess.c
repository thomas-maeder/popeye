#include "conditions/castling_chess.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/castling.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

square castling_partner_origin[toppile+1];
piece castling_partner_kind[toppile+1];
Flags castling_partner_spec[toppile+1];

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type castling_chess_move_player_attack(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_capture>platzwechsel_rochade)
  {
    square const sq_departure = move_gen_top->departure;
    square const sq_arrival = move_gen_top->arrival;
    square const sq_passed = (sq_departure+sq_arrival) / 2;

    jouespec[nbply] = spec[sq_departure];
    jouearr[nbply] = e[sq_departure];

    assert(sq_arrival!=nullsquare);

    pjoue[nbply] = e[sq_departure];

    castling_partner_origin[coup_id] = sq_capture-maxsquare;
    castling_partner_kind[coup_id] = e[castling_partner_origin[coup_id]];
    castling_partner_spec[coup_id] = spec[castling_partner_origin[coup_id]];

    e[sq_passed] = castling_partner_kind[coup_id];
    spec[sq_passed] = castling_partner_spec[coup_id];

    e[castling_partner_origin[coup_id]] = vide;
    CLEARFL(spec[castling_partner_origin[coup_id]]);

    e[sq_departure] = vide;
    spec[sq_departure]= 0;

    e[sq_arrival] = jouearr[nbply];
    spec[sq_arrival] = jouespec[nbply];

    pprise[nbply] = vide;
    pprispec[nbply] = 0;

    result = attack(slices[si].next2,n);

    e[castling_partner_origin[current_move[nbply]]] = e[sq_passed];
    spec[castling_partner_origin[current_move[nbply]]] = spec[sq_passed];

    e[sq_passed] = vide;
    CLEARFL(spec[sq_passed]);

    e[sq_arrival] = vide;
    spec[sq_arrival] = 0;

    e[sq_departure] = pjoue[nbply];
    spec[sq_departure] = jouespec[nbply];
  }
  else
  {
    castling_partner_origin[coup_id] = initsquare;
    result = attack(slices[si].next1,n);
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
stip_length_type castling_chess_move_player_defend(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_capture>platzwechsel_rochade)
  {
    square const sq_departure = move_gen_top->departure;
    square const sq_arrival = move_gen_top->arrival;
    square const sq_passed = (sq_departure+sq_arrival) / 2;

    jouespec[nbply] = spec[sq_departure];
    jouearr[nbply] = e[sq_departure];

    assert(sq_arrival!=nullsquare);

    pjoue[nbply] = e[sq_departure];

    castling_partner_origin[coup_id] = sq_capture-maxsquare;
    castling_partner_kind[coup_id] = e[castling_partner_origin[coup_id]];
    castling_partner_spec[coup_id] = spec[castling_partner_origin[coup_id]];

    e[sq_passed] = castling_partner_kind[coup_id];
    spec[sq_passed] = castling_partner_spec[coup_id];

    e[castling_partner_origin[coup_id]] = vide;
    CLEARFL(spec[castling_partner_origin[coup_id]]);

    e[sq_departure] = vide;
    spec[sq_departure]= 0;

    e[sq_arrival] = jouearr[nbply];
    spec[sq_arrival] = jouespec[nbply];

    pprise[nbply] = vide;
    pprispec[nbply] = 0;

    result = defend(slices[si].next2,n);

    e[castling_partner_origin[current_move[nbply]]] = e[sq_passed];
    spec[castling_partner_origin[current_move[nbply]]] = spec[sq_passed];

    e[sq_passed] = vide;
    CLEARFL(spec[sq_passed]);

    e[sq_arrival] = vide;
    spec[sq_arrival] = 0;

    e[sq_departure] = pjoue[nbply];
    spec[sq_departure] = jouespec[nbply];
  }
  else
  {
    castling_partner_origin[coup_id] = initsquare;
    result = defend(slices[si].next1,n);
  }

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
stip_length_type castling_chess_king_square_adjuster_attack(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (castling_partner_origin[coup_id]!=initsquare
      && king_square[advers(slices[si].starter)]==castling_partner_origin[coup_id])
  {
    square const sq_departure = move_generation_stack[coup_id].departure;
    square const sq_arrival = move_generation_stack[coup_id].arrival;
    square const sq_passed = (sq_departure+sq_arrival) / 2;
    king_square[advers(slices[si].starter)] = sq_passed;
    result = attack(slices[si].next1,n);
    king_square[advers(slices[si].starter)] = castling_partner_origin[coup_id];
  }
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
stip_length_type castling_chess_king_square_adjuster_defend(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (castling_partner_origin[coup_id]!=initsquare
      && king_square[advers(slices[si].starter)]==castling_partner_origin[coup_id])
  {
    square const sq_departure = move_generation_stack[coup_id].departure;
    square const sq_arrival = move_generation_stack[coup_id].arrival;
    square const sq_passed = (sq_departure+sq_arrival) / 2;
    king_square[advers(slices[si].starter)] = sq_passed;
    result = defend(slices[si].next1,n);
    king_square[advers(slices[si].starter)] = castling_partner_origin[coup_id];
  }
  else
    result = defend(slices[si].next1,n);

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
stip_length_type castling_chess_haan_chess_partner_square_blocker_attack(slice_index si,
                                                                         stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  e[castling_partner_origin[current_move[nbply]]] = obs;
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
stip_length_type castling_chess_haan_chess_partner_square_blocker_defend(slice_index si,
                                                                         stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  e[castling_partner_origin[current_move[nbply]]] = obs;
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_castling_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  insert_alternative_move_players(si,STCastlingChessMovePlayer);
  stip_instrument_moves(si,STCastlingChessKingSquareAdjuster);

  if (CondFlag[haanerchess])
    stip_instrument_moves(si,STCastlingChessHaanPartnerSquareBlocker);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
