#include "conditions/geneva.h"
#include "conditions/anticirce/anticirce.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

circe_variant_type geneva_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void geneva_reset_variant(circe_variant_type *variant)
{
  anticirce_reset_variant(variant);

  variant->do_place_reborn = false;
  variant->is_rex_inclusive = false;
  variant->on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_strict;
}

/* Control a Geneva variant for meaningfulness
 * @param variant address of the variant
 */
boolean geneva_is_variant_consistent(circe_variant_type const *variant)
{
  if (variant->on_occupied_rebirth_square_default!=circe_on_occupied_rebirth_square_strict)
    return false;
  if (variant->reborn_walk_adapter!=circe_reborn_walk_adapter_none)
    return false;
  if (variant->is_turncoat)
    return false;
  if (variant->default_relevant_piece!=circe_relevant_piece_capturer)
    return false;
  if (variant->actual_relevant_piece!=circe_relevant_piece_capturer)
    return false;
  if (variant->relevant_capture!=circe_relevant_capture_thismove)
    return false;
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_pwc)
    return false;
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_equipollents)
    return false;
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_take_and_make)
    return false;

  return true;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void geneva_initialise_reborn_from_capturer_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  move_effect_journal_index_type const base = move_effect_journal_base[context->relevant_ply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->relevant_square = move_effect_journal[movement].u.piece_movement.from;
  context->rebirth_from = context->relevant_square;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void geneva_stop_catpure_from_rebirth_square_solve(slice_index si)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  move_effect_journal_index_type const base = move_effect_journal_base[context->relevant_ply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,
                            move_effect_journal[movement].type==move_effect_piece_movement
                            && move_effect_journal[movement].u.piece_movement.from==context->rebirth_square);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Geneva Chess
 * @param si identifies the root slice of the stipulation
 */
void geneva_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  circe_initialise_solving(si,&geneva_variant,STMove,&move_insert_slices,STGenevaConsideringRebirth);

  circe_instrument_solving(si,
                           STGenevaConsideringRebirth,
                           STCirceDeterminingRebirth,
                           alloc_pipe(STGenevaInitialiseRebornFromCapturer));
  circe_instrument_solving(si,
                           STGenevaConsideringRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STGenevaStopCaptureFromRebirthSquare));

  stip_instrument_check_validation(si,
                                   nr_sides,
                                   STValidateCheckMoveByPlayingCapture);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
