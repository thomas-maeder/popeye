#include "conditions/messigny.h"
#include "solving/castling.h"
#include "solving/move_effect_journal.h"
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

    pjoue[nbply] = pi_moving;
    pprise[nbply] = e[sq_arrival];
    pprispec[nbply] = spec[sq_arrival];

    move_effect_journal_do_piece_exchange(move_effect_reason_messigny_exchange,
                                          sq_arrival,sq_departure);

    result = attack(slices[si].next2,n);
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

    pjoue[nbply] = pi_moving;
    pprise[nbply] = e[sq_arrival];
    pprispec[nbply] = spec[sq_arrival];

    move_effect_journal_do_piece_exchange(move_effect_reason_messigny_exchange,
                                          sq_arrival,sq_departure);

    result = defend(slices[si].next2,n);
  }
  else
    result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
