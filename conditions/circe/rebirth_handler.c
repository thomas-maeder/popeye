#include "conditions/circe/rebirth_handler.h"
#include "conditions/circe/capture_fork.h"
#include "pydata.h"
#include "conditions/einstein/einstein.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "pieces/side_change.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* Execute a Circe rebirth.
 * This is a helper function for alternative Circe types
 * @param sq_rebirth rebirth square
 * @param pi_reborn type of piece to be reborn
 * @param spec_reborn flags of the piece to be reborn
 */
void circe_do_rebirth(square sq_rebirth, piece pi_reborn, Flags spec_reborn)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_rebirth);
  TracePiece(pi_reborn);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_addition(move_effect_reason_circe_rebirth,
                                        sq_rebirth,
                                        pi_reborn,
                                        spec_reborn);
  current_circe_rebirth_square[nbply] = sq_rebirth;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void try_rebirth(Side trait_ply)
{
  square const pi_captured = pprise[nbply];
  square const pi_departing = pjoue[nbply];
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_arrival = move_gen_top->arrival;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  Flags const spec_pi_moving = spec[sq_arrival];
  Flags spec_pi_captured = pprispec[nbply];
  piece const pi_arriving = e[sq_arrival];
  square const prev_rb = prev_king_square[White][nbply];
  square const prev_rn = prev_king_square[Black][nbply];
  square sq_rebirth;
  piece pi_reborn;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (anyclone
      && sq_departure!=prev_rn && sq_departure!=prev_rb)
    /* Circe Clone - new implementation
    ** captured pieces are reborn as pieces
    ** of the same type as the capturing piece
    ** if the latter one is not royal.
    */
    /* change type of pieces according to colour */
    pi_reborn = ((pi_departing * pi_captured < 0)
                 ? -pi_departing
                 : pi_departing);
    /* If it is a pawn give it the pawn-attribut.
    ** Otherwise delete it - the captured piece may
    ** have been a pawn, but is not anymore.
    */
  else if (anytraitor)
  {
    pi_reborn = -pi_captured;
    spec_change_side(&spec_pi_captured);
  }
  else if (CondFlag[chamcirce])
    pi_reborn= ChamCircePiece(pi_captured);
  else if (CondFlag[antieinstein])
    pi_reborn= einstein_increase_piece(pi_captured);
  else
    pi_reborn= pi_captured;

  if (CondFlag[couscous])
    sq_rebirth = (*circerenai)(nbply,
                               pi_arriving,
                               spec_pi_moving,
                               sq_capture,
                               sq_departure,
                               sq_arrival,
                               advers(trait_ply));
  else
    sq_rebirth = (*circerenai)(nbply,
                               pi_reborn,
                               spec_pi_captured,
                               sq_capture,
                               sq_departure,
                               sq_arrival,
                               trait_ply);

  if (CondFlag[contactgrid] && nogridcontact(sq_rebirth))
    sq_rebirth = initsquare;

  if (e[sq_rebirth]==vide)
    circe_do_rebirth(sq_rebirth,pi_reborn,spec_pi_captured);
  else
    current_circe_rebirth_square[nbply] = initsquare;

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
stip_length_type circe_rebirth_handler_attack(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  try_rebirth(slices[si].starter);
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
stip_length_type circe_rebirth_handler_defend(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  try_rebirth(slices[si].starter);
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves_no_replay(si,STCirceRebirthHandler);
  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
