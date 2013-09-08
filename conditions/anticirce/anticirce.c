#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/capture_fork.h"
#include "conditions/circe/circe.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

VariantType AntiCirceType;

/* Find the Circe rebirth effect in the current move
 * @return the index of the rebirth effect
 *         move_effect_journal_base[nbply+1] if there is none
 */
move_effect_journal_index_type anticirce_find_current_rebirth(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (result = base+move_effect_journal_index_offset_other_effects; result<top; ++result)
    if (move_effect_journal[result].type==move_effect_piece_readdition
        && (move_effect_journal[result].reason==move_effect_reason_anticirce_rebirth
            || move_effect_journal[result].reason==move_effect_reason_antisupercirce_rebirth))
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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
stip_length_type anticirce_determine_reborn_piece_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

    context->reborn_walk = get_walk_of_piece_on_square(pos);
    context->reborn_spec = spec[pos];

    /* TODO WinChloe uses the arrival square, which seems to make more sense */
    context->relevant_square = move_effect_journal[capture].u.piece_removal.from;

    context->relevant_walk = context->reborn_walk;
    context->relevant_spec = context->reborn_spec;
    context->relevant_side = advers(slices[si].starter);
  }

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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type anticirce_determine_relevant_piece_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type anticirce_determine_rebirth_square_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  context->rebirth_square = rennormal(context->relevant_walk,
                                      context->relevant_spec,
                                      context->relevant_square,
                                      context->relevant_side);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise solving in Anticirce
 * @param si identifies root slice of stipulation
 */
void anticirce_initialise_solving(slice_index si,
                                  slice_type type_determine_rebirth_square)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticirceDetermineRebornPiece);
  stip_instrument_moves(si,type_determine_rebirth_square);
  stip_insert_anticirce_capture_forks(si);

  stip_instrument_check_validation(si,
                                   nr_sides,
                                   STValidateCheckMoveByPlayingCapture);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_type const * const type = st->param;
    slice_index const prototype = alloc_pipe(*type);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the Anticirce solving machinery with some slice
 * @param si identifies root slice of stipulation
 * @param type slice type of which to add instances
 */
void anticirce_instrument_solving(slice_index si, slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,STAnticirceDetermineRebornPiece,&instrument);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
