#include "conditions/circe/clone.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "conditions/circe/circe.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_clone_determine_reborn_piece_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  PieNam const pi_captured = abs(move_effect_journal[capture].u.piece_removal.removed);
  PieNam const pi_departing = abs(move_effect_journal[movement].u.piece_movement.moving);
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const prev_rb = prev_king_square[White][nbply];
  square const prev_rn = prev_king_square[Black][nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  current_circe_reborn_spec[nbply] = move_effect_journal[capture].u.piece_removal.removedspec;

  if (sq_departure!=prev_rn && sq_departure!=prev_rb)
    current_circe_reborn_piece[nbply] = pi_departing;
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
