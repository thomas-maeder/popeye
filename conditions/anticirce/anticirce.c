#include "conditions/anticirce/anticirce.h"
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

#include "debugging/assert.h"

circe_variant_type anticirce_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void anticirce_reset_variant(circe_variant_type *variant)
{
  variant->is_promotion_possible = false;
  variant->rebirth_reason = move_effect_reason_rebirth_no_choice;
  variant->is_mirror = false;
  variant->is_diametral = false;
  variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_default;
  variant->on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_strict;
  variant->reborn_walk_adapter = circe_reborn_walk_adapter_none;
  variant->is_turncoat = false;
  variant->relevant_piece = circe_relevant_piece_default;
  variant->relevant_capture = circe_relevant_capture_thismove;
  variant->determine_rebirth_square = circe_determine_rebirth_square_from_pas;
  variant->is_frischauf = false;
  variant->anticirce_type = anticirce_type_count;
  variant->is_rex_inclusive = true;
}

/* Initialise the Anticirce machinery from the capture in a particular ply
 * @param ply identifies the ply
 */
void anticirce_initialise_from_capture_in_ply(ply ply)
{
  move_effect_journal_index_type const base = move_effect_journal_base[ply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
  PieceIdType const moving_id = GetPieceId(movingspec);
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  context->rebirth_from = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                                 moving_id,
                                                                                 sq_arrival);
  context->reborn_walk = get_walk_of_piece_on_square(context->rebirth_from);
  context->reborn_spec = spec[context->rebirth_from];

  /* TODO WinChloe uses the arrival square, which seems to make more sense */
  context->relevant_square = move_effect_journal[capture].u.piece_removal.from;

  context->relevant_walk = context->reborn_walk;
  context->relevant_spec = context->reborn_spec;
  context->relevant_side = advers(trait[ply]);
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
stip_length_type anticirce_initialise_from_current_capture_solve(slice_index si,
                                                                 stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  anticirce_initialise_from_capture_in_ply(nbply);

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
stip_length_type anticirce_remove_capturer_solve(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              GetPieceId(context->relevant_spec),
                                                                              context->rebirth_from);
    move_effect_journal_do_piece_removal(move_effect_reason_transfer_no_choice,
                                         pos);
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
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type anticirce_place_reborn_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_readdition(anticirce_variant.rebirth_reason,
                                          context->rebirth_square,
                                          context->reborn_walk,
                                          context->reborn_spec);
  ++circe_rebirth_context_stack_pointer;
  result = solve(slices[si].next1,n);
  --circe_rebirth_context_stack_pointer;

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

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STAnticirceConsideringRebirth),
        alloc_pipe(STAnticirceDeterminingRebornPiece),
        alloc_pipe(STAnticirceRemoveCapturer),
        alloc_pipe(STAnticircePlacingReborn),
        alloc_pipe(STAnticircePlaceReborn)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Anticirce
 * @param si identifies root slice of stipulation
 */
void anticirce_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceNonCapturingMoveFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

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
  stip_structure_traversal_override_single(&st,
                                           STAnticirceDeterminingRebornPiece,
                                           &instrument);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
