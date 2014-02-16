#include "conditions/anticirce/relaxed.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/circe/circe.h"
#include "solving/move_effect_journal.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
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
stip_length_type anticirce_place_reborn_relaxed_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    move_effect_journal_do_piece_removal(move_effect_reason_kamikaze_capturer,
                                         pos);
  }

  if (is_square_empty(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square))
  {
    move_effect_journal_do_piece_readdition(move_effect_reason_rebirth_no_choice,
                                            circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square,
                                            context->reborn_walk,
                                            context->reborn_spec);
    ++circe_rebirth_context_stack_pointer;
    result = solve(slices[si].next1,n);
    --circe_rebirth_context_stack_pointer;
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Override the Circe instrumentation of the solving machinery with
 * relaxed (i.e. non-strict) Anticirce
 * @param si identifies root slice of stipulation
 */
void anticirce_relaxed_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticircePlaceRebornRelaxed);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
