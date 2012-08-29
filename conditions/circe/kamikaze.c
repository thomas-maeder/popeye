#include "conditions/circe/kamikaze.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static void do_kamikaze_rebirth(Side trait_ply)
{
  square const pi_captured = pprise[nbply];
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_arrival = move_gen_top->arrival;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  Flags const spec_pi_moving = spec[sq_arrival];
  Flags const spec_pi_captured = pprispec[nbply];
  piece const pi_arriving = e[sq_arrival];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec_pi_moving,Kamikaze) && pi_captured!=vide)
  {
    if (CondFlag[couscous])
      current_anticirce_rebirth_square[nbply] = (*circerenai)(nbply,
                                                              pi_captured,
                                                              spec_pi_captured,
                                                              sq_capture,
                                                              sq_departure,
                                                              sq_arrival,
                                                              trait_ply);
    else
      current_anticirce_rebirth_square[nbply] = (*circerenai)(nbply,
                                                              pi_arriving,
                                                              spec_pi_moving,
                                                              sq_capture,
                                                              sq_departure,
                                                              sq_arrival,
                                                              advers(trait_ply));

    move_effect_journal_do_piece_removal(move_effect_reason_kamikaze_capturer,
                                         sq_arrival);

    if (e[current_anticirce_rebirth_square[nbply]]==vide
        && !(CondFlag[contactgrid] && nogridcontact(current_anticirce_rebirth_square[nbply])))
      move_effect_journal_do_piece_addition(move_effect_reason_anticirce_rebirth,
                                            current_anticirce_rebirth_square[nbply],
                                            pi_arriving,
                                            spec_pi_moving);
    else
      current_anticirce_rebirth_square[nbply] = initsquare;
  }
  else
    current_anticirce_rebirth_square[nbply] = initsquare;

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
stip_length_type circe_kamikaze_rebirth_handler_attack(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  do_kamikaze_rebirth(slices[si].starter);
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
stip_length_type circe_kamikaze_rebirth_handler_defend(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  do_kamikaze_rebirth(slices[si].starter);
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_kamikaze_rebirth_handlers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves_no_replay(si,STCirceKamikazeRebirthHandler);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
