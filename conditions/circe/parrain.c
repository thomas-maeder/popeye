#include "conditions/circe/parrain.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/conditions.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/fork.h"
#include "solving/binary.h"
#include "position/piece_movement.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "position/piece_creation.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

retro_capture_type retro_capture;

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
void circe_parrain_undo_retro_capture(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_movement(move_effect_reason_diagram_setup,
                                        retro_capture.on,
                                        retro_capture_departure);
  move_effect_journal_do_piece_creation(move_effect_reason_diagram_setup,
                                        retro_capture.on,
                                        retro_capture.walk,
                                        retro_capture.flags,
                                        no_side);

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
void circe_parrain_redo_retro_capture(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_do_capture_move(retro_capture_departure,
                                      retro_capture.on,
                                      retro_capture.on,
                                      move_effect_reason_regular_capture);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Circe Parrain (apart from the rebirth
 * square determination, whose instrumentation is elsewhere)
 * @param si identifies entry slice into solving machinery
 * @param variant identifies address of structure holding the Circe variant
 * @param interval_start type of slice that starts the sequence of slices
 *                       implementing that variant
 */
void circe_solving_instrument_parrain(slice_index si,
                                      struct circe_variant_type const *variant,
                                      slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  circe_insert_rebirth_avoider(si,
                               interval_start,
                               interval_start,
                               alloc_fork_slice(STCirceParrainThreatFork,
                                                no_slice),
                               STCirceRebirthAvoided,
                               STCirceDoneWithRebirth);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int move_vector(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;
  int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply]<=top);

  for (curr = base; curr!=top; ++curr)
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
void circe_parrain_determine_rebirth_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->rebirth_square = context->relevant_square+move_vector();

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
void circe_contraparrain_determine_rebirth_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->rebirth_square = context->relevant_square-move_vector();

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
void circe_parrain_threat_fork_solve(slice_index si)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  move_effect_journal_index_type const base = move_effect_journal_base[context->relevant_ply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  binary_solve_if_then_else(si,capture>=move_effect_journal_base[context->relevant_ply+1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
