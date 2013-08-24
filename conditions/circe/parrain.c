#include "conditions/circe/parrain.h"
#include "conditions/conditions.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "conditions/circe/circe.h"
#include "debugging/trace.h"

#include <assert.h>

static int move_vector(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;
  int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply]<=top);

  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement)
    {
      square const from = move_effect_journal[curr].u.piece_movement.from;
      square const to = move_effect_journal[curr].u.piece_movement.to;
      result += to-from;
    }

  if (CondFlag[contraparrain])
    result = -result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type circe_parrain_determine_rebirth_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
  move_effect_journal_index_type const parent_capture = parent_base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (parent_capture>=move_effect_journal_base[parent+1] /* threat! */
      || move_effect_journal[parent_capture].type==move_effect_no_piece_removal)
  {
    current_circe_rebirth_square[nbply] = initsquare;
    result = solve(slices[si].next1,n);
  }
  else
  {
    current_circe_rebirth_square[nbply] = move_effect_journal[parent_capture].u.piece_removal.from+move_vector();
    current_circe_reborn_piece[nbply] = move_effect_journal[parent_capture].u.piece_removal.removed;
    current_circe_reborn_spec[nbply] = move_effect_journal[parent_capture].u.piece_removal.removedspec;
    current_circe_rebirth_reason[nbply] = move_effect_reason_circe_rebirth;
    result = solve(slices[si].next1,n);
    current_circe_rebirth_reason[nbply] = move_effect_no_reason;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_parrain_rebirth_handlers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceParrainDetermineRebirth);
  stip_instrument_moves(si,STCircePlaceReborn);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
