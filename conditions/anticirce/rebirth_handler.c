#include "conditions/anticirce/rebirth_handler.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>

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

    current_anticirce_rebirth_square[nbply] = (*antirenai)(pi_arriving,
                                                           spec_pi_moving,
                                                           sq_capture,
                                                           sq_departure,
                                                           sq_arrival,
                                                           advers(trait_ply));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_rebirth_handler_solve(slice_index si,
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
      result = solve(slices[si].next1,n);
    else if ((!AntiCirCheylan
              && sq_rebirth==move_generation_stack[current_move[nbply]].arrival)
             || e[sq_rebirth]==vide)
    {
      anticirce_do_rebirth(move_effect_reason_anticirce_rebirth);
      result = solve(slices[si].next1,n);
    }
    else
      result = slack_length-2;
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
