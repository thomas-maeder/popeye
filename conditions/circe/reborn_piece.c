#include "conditions/circe/reborn_piece.h"
#include "conditions/circe/circe.h"
#include "position/position.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Determine the reborn walk adapter (if any) of a Circe variant
 * @param variant address of the structure representing the variant
 * @return adapter slice type; no_slice_type if no adapter is required
 */
static slice_type get_reborn_walk_adapter(circe_variant_type const *variant)
{
  slice_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (variant->reborn_walk_adapter)
  {
    case circe_reborn_walk_adapter_none:
      result = no_slice_type;
      break;

    case circe_reborn_walk_adapter_clone:
      result = (variant->default_relevant_piece==circe_relevant_piece_capturer
                ? STAntiCloneCirceDetermineRebornWalk
                : STCirceCloneDetermineRebornWalk);
      break;

    case circe_reborn_walk_adapter_chameleon:
      result = STChameleonCirceAdaptRebornWalk;
      break;

    case circe_reborn_walk_adapter_einstein:
       result = (variant->default_relevant_piece==circe_relevant_piece_capturer
                 ? STCirceReverseEinsteinAdjustRebornWalk
                 : STCirceEinsteinAdjustRebornWalk);
      break;

    case circe_reborn_walk_adapter_reversaleinstein:
      result = (variant->default_relevant_piece==circe_relevant_piece_capturer
                ? STCirceEinsteinAdjustRebornWalk
                : STCirceReverseEinsteinAdjustRebornWalk);
      break;

    default:
      assert(0);
      result = no_slice_type;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(slice_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

static slice_type get_reborn_initialiser_type(circe_variant_type const *variant)
{
  slice_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (variant->default_relevant_piece)
  {
    case circe_relevant_piece_capturee:
      result = STCirceInitialiseRebornFromCapturee;
      break;

    case circe_relevant_piece_capturer:
      result = STAnticirceInitialiseRebornFromCapturer;
      break;

    case circe_relevant_piece_generated:
      result = STMarscirceInitialiseRebornFromGenerated;
      break;

    case circe_relevant_piece_observing_walk:
      result = STMarsIterateObservers;
      break;

    default:
      assert(0);
      result = no_slice_type;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(slice_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery with the logic for determining which piece
 * is reborn in a Circe variant
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_reborn_piece(slice_index si,
                                           circe_variant_type const *variant,
                                           slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",interval_start);
  TraceFunctionParamListEnd();

  circe_instrument_solving(si,
                           interval_start,
                           STCirceDeterminingRebirth,
                           alloc_pipe(get_reborn_initialiser_type(variant)));

  {
    slice_type const adapter_type = get_reborn_walk_adapter(variant);
    if (adapter_type!=no_slice_type)
      circe_instrument_solving(si,
                               interval_start,
                               STCirceDeterminingRebirth,
                               alloc_circe_handler_slice(adapter_type,variant));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the reborn walk adapter of a Circe variant object
 * @param adapter the overrider
 * @return true if the adapter hasn't been overridden yet
 */
boolean circe_override_reborn_walk_adapter(circe_variant_type *variant,
                                           circe_reborn_walk_adapter_type adapter)
{
  boolean result;

  if (variant->reborn_walk_adapter==circe_reborn_walk_adapter_none)
  {
    variant->reborn_walk_adapter = adapter;
    result = true;
  }
  else
    result = false;

  return result;
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
void circe_initialise_reborn_from_capturee_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  move_effect_journal_index_type const base = move_effect_journal_base[context->relevant_ply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* circe capture fork makes sure of that */
  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  context->reborn_walk = move_effect_journal[capture].u.piece_removal.walk;
  context->reborn_spec = move_effect_journal[capture].u.piece_removal.flags;
  context->relevant_square = move_effect_journal[capture].u.piece_removal.on;
  context->relevant_side = trait[context->relevant_ply];
  context->rebirth_as = advers(trait[context->relevant_ply]);

  pipe_dispatch_delegate(si);

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
void circe_initialise_reborn_from_capturer_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  move_effect_journal_index_type const base = move_effect_journal_base[context->relevant_ply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
  PieceIdType const moving_id = GetPieceId(movingspec);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->rebirth_from = move_effect_journal_follow_piece_through_other_effects(context->relevant_ply,
                                                                                 moving_id,
                                                                                 sq_arrival);
  context->reborn_walk = get_walk_of_piece_on_square(context->rebirth_from);
  context->reborn_spec = being_solved.spec[context->rebirth_from];

  /* TODO WinChloe uses the arrival square, which seems to make more sense */
  context->relevant_square = move_effect_journal[capture].u.piece_removal.on;
  context->relevant_side = advers(trait[context->relevant_ply]);

  context->rebirth_as = trait[context->relevant_ply];

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
