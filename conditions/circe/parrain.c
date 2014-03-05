#include "conditions/circe/parrain.h"
#include "conditions/conditions.h"
#include "conditions/circe/circe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "solving/move_effect_journal.h"
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
stip_length_type circe_parrain_initalise_from_capture_in_last_move_solve(slice_index si,
                                                                         stip_length_type n)
{
  stip_length_type result;
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
  move_effect_journal_index_type const parent_capture = parent_base+move_effect_journal_index_offset_capture;
  move_effect_journal_entry_type const * const capture = &move_effect_journal[parent_capture];
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  context->reborn_walk = capture->u.piece_removal.removed;
  context->reborn_spec = capture->u.piece_removal.removedspec;

  context->relevant_square = capture->u.piece_removal.from;
  context->relevant_walk = context->reborn_walk;
  context->relevant_spec = context->reborn_spec;

  context->relevant_side = advers(slices[si].starter);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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

  /* TODO we have to solve the enpassant capture / locust capture problem of
   * Circe Equipollents in the future. */
#if defined(WINCHLOE)
  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = sq_capture + move_effect_journal[movement].u.piece_movement.to - sq_departure;
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type circe_parrain_determine_rebirth_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  context->rebirth_square = context->relevant_square+move_vector();

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type circe_contraparrain_determine_rebirth_solve(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  context->rebirth_square = context->relevant_square-move_vector();

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
stip_length_type circe_parrain_capture_fork_solve(slice_index si,
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
    result = solve(slices[si].next2,n);
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
