#include "conditions/circe/rebirth_handler.h"
#include "conditions/circe/capture_fork.h"
#include "pydata.h"
#include "conditions/circe/cage.h"
#include "conditions/einstein/einstein.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/legal_move_counter.h"
#include "solving/castling.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pieces/side_change.h"
#include "debugging/trace.h"

#include <assert.h>

/* Execute a king's rebirth in Circe rex. incl. and similar conditions
 * @param sq_from where the king was captured
 * @param sq_rebirth where the king is to be reborn
 */
void do_king_rebirth(square sq_from, square sq_rebirth)
{
  if (sq_from==prev_king_square[White][nbply])
    king_square[White] = sq_rebirth;
  if (sq_from==prev_king_square[Black][nbply])
    king_square[Black] = sq_rebirth;
}

void circe_undo_rebirth(square sq_rebirth)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  assert(sq_rebirth!=initsquare);

  if (sq_rebirth!=sq_arrival)
  {
    --nbpiece[e[sq_rebirth]];
    e[sq_rebirth] = vide;
    spec[sq_rebirth] = 0;
  }

  if (anytraitor)
    spec_change_side(&spec[sq_rebirth]);
}

void circe_do_rebirth(square sq_rebirth, piece pi_reborn, Flags spec_reborn)
{
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_rebirth);
  TracePiece(pi_reborn);
  TraceFunctionParamListEnd();

  if (rex_circe)
    do_king_rebirth(sq_capture,sq_rebirth);
  e[sq_rebirth] = pi_reborn;
  ++nbpiece[pi_reborn];
  spec[sq_rebirth] = spec_reborn;
  restore_castling_rights(sq_rebirth);
  current_circe_rebirth_square[nbply] = sq_rebirth;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square do_rebirth(Side trait_ply)
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
  square result;
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
    result = (*circerenai)(nbply,
                               pi_arriving,
                               spec_pi_moving,
                               sq_capture,
                               sq_departure,
                               sq_arrival,
                               advers(trait_ply));
  else
    result = (*circerenai)(nbply,
                               pi_reborn,
                               spec_pi_captured,
                               sq_capture,
                               sq_departure,
                               sq_arrival,
                               trait_ply);

  if (CondFlag[contactgrid] && nogridcontact(result))
    result = initsquare;

  if (e[result]==vide)
    circe_do_rebirth(result,pi_reborn,spec_pi_captured);
  else
    result = initsquare;

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
stip_length_type circe_rebirth_handler_attack(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const sq_rebirth = do_rebirth(slices[si].starter);
    if (sq_rebirth==initsquare)
      result = attack(slices[si].next1,n);
    else
    {
      current_circe_rebirth_square[nbply] = sq_rebirth;
      result = attack(slices[si].next1,n);
      circe_undo_rebirth(sq_rebirth);
      current_circe_rebirth_square[nbply] = initsquare;
    }
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
stip_length_type circe_rebirth_handler_defend(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    square const sq_rebirth = do_rebirth(slices[si].starter);
    if (sq_rebirth==initsquare)
      result = defend(slices[si].next1,n);
    else
    {
      current_circe_rebirth_square[nbply] = sq_rebirth;
      result = defend(slices[si].next1,n);
      circe_undo_rebirth(sq_rebirth);
      current_circe_rebirth_square[nbply] = initsquare;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STCirceRebirthHandler);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_rebirth_handlers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STIsardamDefenderFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
