#include "conditions/castling_chess.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/castling.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
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

    assert(sq_arrival!=nullsquare);

    jouespec[nbply] = spec[sq_departure];
    pjoue[nbply] = e[sq_departure];
    pprise[nbply] = vide;
    pprispec[nbply] = 0;

    castling_partner_origin[coup_id] = sq_capture-maxsquare;
    castling_partner_kind[coup_id] = e[castling_partner_origin[coup_id]];
    castling_partner_spec[coup_id] = spec[castling_partner_origin[coup_id]];

    move_effect_journal_do_piece_movement(move_effect_reason_moving_piece_movement,
                                          sq_departure,sq_arrival);
    move_effect_journal_do_piece_movement(move_effect_reason_castling_partner_movement,
                                          castling_partner_origin[coup_id],
                                          sq_passed);

    result = attack(slices[si].next2,n);
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

    assert(sq_arrival!=nullsquare);

    jouespec[nbply] = spec[sq_departure];
    pjoue[nbply] = e[sq_departure];
    pprise[nbply] = vide;
    pprispec[nbply] = 0;

    castling_partner_origin[coup_id] = sq_capture-maxsquare;
    castling_partner_kind[coup_id] = e[castling_partner_origin[coup_id]];
    castling_partner_spec[coup_id] = spec[castling_partner_origin[coup_id]];

    move_effect_journal_do_piece_movement(move_effect_reason_moving_piece_movement,
                                          sq_departure,sq_arrival);
    move_effect_journal_do_piece_movement(move_effect_reason_castling_partner_movement,
                                          castling_partner_origin[coup_id],
                                          sq_passed);

    result = defend(slices[si].next2,n);
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

/* Instrument slices with move tracers
 */
void stip_insert_castling_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  insert_alternative_move_players_no_replay(si,STCastlingChessMovePlayer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
