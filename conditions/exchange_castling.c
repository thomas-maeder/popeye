#include "conditions/exchange_castling.h"
#include "conditions/castling_chess.h"
#include "stipulation/stipulation.h"
#include "solving/castling.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type exchange_castling_move_player_attack(slice_index si,
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

  if (sq_capture==platzwechsel_rochade)
  {
    Side const trait_ply = trait[nbply];

    square const sq_departure = move_gen_top->departure;
    square const sq_arrival = move_gen_top->arrival;

    jouespec[nbply] = spec[sq_departure];
    jouearr[nbply] = e[sq_departure];

    assert(sq_arrival!=nullsquare);

    pjoue[nbply] = e[sq_departure];

    castling_partner_origin[coup_id] = sq_arrival;
    castling_partner_kind[coup_id] = e[sq_arrival];
    castling_partner_spec[coup_id] = spec[sq_arrival];
    castling_partner_origin[coup_id] = -sq_arrival; /* hack for output */

    e[sq_arrival] = e[sq_departure];
    spec[sq_arrival] = spec[sq_departure];

    e[sq_departure] = castling_partner_kind[coup_id];
    spec[sq_departure] = castling_partner_spec[coup_id];

    platzwechsel_rochade_allowed[trait_ply][nbply] = false;

    pprise[nbply] = vide;
    pprispec[nbply] = 0;

    result = attack(slices[si].next2,n);

    platzwechsel_rochade_allowed[White][nbply] = platzwechsel_rochade_allowed[White][parent_ply[nbply]];
    platzwechsel_rochade_allowed[Black][nbply] = platzwechsel_rochade_allowed[Black][parent_ply[nbply]];

    e[sq_arrival] = e[sq_departure];
    spec[sq_arrival] = spec[sq_departure];

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
stip_length_type exchange_castling_move_player_defend(slice_index si,
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

  if (sq_capture==platzwechsel_rochade)
  {
    Side const trait_ply = trait[nbply];

    square const sq_departure = move_gen_top->departure;
    square const sq_arrival = move_gen_top->arrival;

    jouespec[nbply] = spec[sq_departure];
    jouearr[nbply] = e[sq_departure];

    assert(sq_arrival!=nullsquare);

    pjoue[nbply] = e[sq_departure];

    castling_partner_origin[coup_id] = sq_arrival;
    castling_partner_kind[coup_id] = e[sq_arrival];
    castling_partner_spec[coup_id] = spec[sq_arrival];
    castling_partner_origin[coup_id] = -sq_arrival; /* hack for output */

    e[sq_arrival] = e[sq_departure];
    spec[sq_arrival] = spec[sq_departure];

    e[sq_departure] = castling_partner_kind[coup_id];
    spec[sq_departure] = castling_partner_spec[coup_id];

    platzwechsel_rochade_allowed[trait_ply][nbply] = false;

    pprise[nbply] = vide;
    pprispec[nbply] = 0;

    result = defend(slices[si].next2,n);

    platzwechsel_rochade_allowed[White][nbply] = platzwechsel_rochade_allowed[White][parent_ply[nbply]];
    platzwechsel_rochade_allowed[Black][nbply] = platzwechsel_rochade_allowed[Black][parent_ply[nbply]];

    e[sq_arrival] = e[sq_departure];
    spec[sq_arrival] = spec[sq_departure];

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

/* Instrument slices with move tracers
 */
void stip_insert_exchange_castling(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  insert_alternative_move_players(si,STExchangeCastlingMovePlayer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
