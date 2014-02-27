#include "conditions/circe/equipollents.h"
#include "conditions/circe/circe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_equipollents_determine_rebirth_square_solve(slice_index si,
                                                                   stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* we have to solve the enpassant capture / locust capture problem in the future. */
#if defined(WINCHLOE)
  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = sq_capture + move_effect_journal[movement].u.piece_movement.to - sq_departure;
#endif

  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = sq_capture + sq_capture - sq_departure;
  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_reason = move_effect_reason_rebirth_no_choice;
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
