#include "conditions/circe/relevant_piece.h"
#include "conditions/circe/circe.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static slice_type get_relevant_piece_determinator(circe_variant_type const *variant)
{
  slice_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (variant->actual_relevant_piece==variant->default_relevant_piece)
    result = STCirceInitialiseRelevantFromReborn;
  else if (variant->actual_relevant_piece==circe_relevant_piece_capturer)
    result = STCirceCouscousMakeCapturerRelevant;
  else
    result = STAnticirceCouscousMakeCaptureeRelevant;

  TraceFunctionExit(__func__);
  TraceEnumerator(slice_type,result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery with the logic for determining which piece
 * is relevant for the rebirth in a Circe variant
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_relevant_piece(slice_index si,
                                             struct circe_variant_type const *variant,
                                             slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",interval_start);
  TraceFunctionParamListEnd();

  circe_instrument_solving(si,
                           interval_start,
                           STCirceDeterminingRebirth,
                           alloc_pipe(get_relevant_piece_determinator(variant)));

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
void circe_initialise_relevant_from_reborn(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->relevant_walk = context->reborn_walk;
  context->relevant_spec = context->reborn_spec;

  TraceWalk(context->relevant_walk);
  TraceValue("%u",context->relevant_spec);
  TraceEOL();

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
