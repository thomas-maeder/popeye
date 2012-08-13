#include "conditions/messigny.h"
#include "solving/castling.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"

#include <assert.h>

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_messigny(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_alternative_move_players(si,STMessignyMovePlayer);

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
stip_length_type messigny_move_player_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_capture==messigny_exchange)
  {
    square const sq_arrival = move_gen_top->arrival;
    square const sq_departure = move_gen_top->departure;
    piece const pi_moving = e[sq_departure];
    Flags const spec_pi_moving = spec[sq_departure];

    pjoue[nbply] = pi_moving;
    jouespec[nbply] = spec_pi_moving;

    pprise[nbply] = e[sq_arrival];
    pprispec[nbply] = spec[sq_arrival];

    e[sq_departure] = e[sq_arrival];
    spec[sq_departure] = spec[sq_arrival];

    jouearr[nbply] = pi_moving;
    e[sq_arrival] = pi_moving;

    spec[sq_arrival] = spec_pi_moving;

    if (king_square[White]==sq_departure)
      king_square[White] = sq_arrival;
    else if (king_square[White]==sq_arrival)
      king_square[White] = sq_departure;

    if (king_square[Black]==sq_departure)
      king_square[Black] = sq_arrival;
    else if (king_square[Black]==sq_arrival)
      king_square[Black] = sq_departure;

    result = attack(slices[si].next2,n);

    e[sq_arrival] = e[sq_departure];
    spec[sq_arrival] = spec[sq_departure];

    e[sq_departure] = pi_moving;
    spec[sq_departure] = spec_pi_moving;
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
stip_length_type messigny_move_player_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_capture==messigny_exchange)
  {
    square const sq_arrival = move_gen_top->arrival;
    square const sq_departure = move_gen_top->departure;
    piece const pi_moving = e[sq_departure];
    Flags const spec_pi_moving = spec[sq_departure];

    pjoue[nbply] = pi_moving;
    jouespec[nbply] = spec_pi_moving;

    pprise[nbply] = e[sq_arrival];
    pprispec[nbply] = spec[sq_arrival];

    e[sq_departure] = e[sq_arrival];
    spec[sq_departure] = spec[sq_arrival];

    jouearr[nbply] = pi_moving;
    e[sq_arrival] = pi_moving;

    spec[sq_arrival] = spec_pi_moving;

    if (king_square[White]==sq_departure)
      king_square[White] = sq_arrival;
    else if (king_square[White]==sq_arrival)
      king_square[White] = sq_departure;

    if (king_square[Black]==sq_departure)
      king_square[Black] = sq_arrival;
    else if (king_square[Black]==sq_arrival)
      king_square[Black] = sq_departure;

    result = defend(slices[si].next2,n);

    e[sq_arrival] = e[sq_departure];
    spec[sq_arrival] = spec[sq_departure];

    e[sq_departure] = pi_moving;
    spec[sq_departure] = spec_pi_moving;
  }
  else
    result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
