#include "conditions/circe/clone.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "conditions/circe/circe.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_clone_determine_reborn_piece_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;
  square const pi_captured = pprise[nbply];
  square const pi_departing = pjoue[nbply];
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_departure = move_gen_top->departure;
  square const prev_rb = prev_king_square[White][nbply];
  square const prev_rn = prev_king_square[Black][nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_circe_reborn_spec[nbply] = pprispec[nbply];

  if (sq_departure!=prev_rn && sq_departure!=prev_rb)
    /* Circe Clone - new implementation
    ** captured pieces are reborn as pieces
    ** of the same type as the capturing piece
    ** if the latter one is not royal.
    */
    /* change type of pieces according to colour */
    current_circe_reborn_piece[nbply] = ((pi_departing * pi_captured < 0)
                                         ? -pi_departing
                                         : pi_departing);
    /* If it is a pawn give it the pawn-attribut.
    ** Otherwise delete it - the captured piece may
    ** have been a pawn, but is not anymore.
    */
  else
    current_circe_reborn_piece[nbply] = pi_captured;

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_clone(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_replace_circe_determine_reborn_piece(si,STCirceCloneDetermineRebornPiece);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
