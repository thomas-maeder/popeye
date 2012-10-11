#include "conditions/circe/assassin.h"
#include "conditions/circe/rebirth_handler.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "conditions/circe/rebirth_handler.h"
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
    result = rennormal(pi_captured,
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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_assassin_rebirth_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_circe_rebirth_square[nbply] = determine_rebirth_square(slices[si].starter);
  if (current_circe_rebirth_square[nbply]==initsquare)
  {
    current_circe_reborn_piece[nbply] = vide;
    result = solve(slices[si].next1,n);
  }
  else if (e[current_circe_rebirth_square[nbply]]==vide)
  {
    current_circe_reborn_piece[nbply] = pprise[nbply];
    current_circe_reborn_spec[nbply] = pprispec[nbply];
    circe_do_rebirth(move_effect_reason_circe_rebirth);
    result = solve(slices[si].next1,n);
  }
  else if (current_circe_rebirth_square[nbply]==king_square[slices[si].starter])
    result = slack_length-2;
  else
  {
    move_effect_journal_do_piece_removal(move_effect_reason_assassin_circe_rebirth,
                                         current_circe_rebirth_square[nbply]);
    current_circe_reborn_piece[nbply] = pprise[nbply];
    current_circe_reborn_spec[nbply] = pprispec[nbply];
    circe_do_rebirth(move_effect_reason_circe_rebirth);
    result = solve(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
