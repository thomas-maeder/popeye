#include "conditions/anticirce/capture_exchange.h"
#include "position/position.h"
#include "position/effects/piece_movement.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rex_inclusive.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/temporary_hacks.h"
#include "solving/post_move_iteration.h"
#include "solving/single_piece_move_generator.h"
#include "solving/move_generator.h"
#include "solving/conditional_pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

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
void anticirce_capture_exchange_collect_rebirth_squares_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

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
void anti_circe_capture_exchange_determine_rebirth_squares_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  context->rebirth_square = square_d4;

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
void anti_circe_capture_exchange_do_transfer_solve(slice_index si)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_movement(SLICE_U(si).circe_handler.variant->rebirth_reason,
                                        context->rebirth_square,
                                        context->rebirth_from);
  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the solving machinery with Anticirce Capture&Exchange
 * @param si identifies root slice of stipulation
 * @param interval_start start of the slices interval to be initialised
 */
void anticirce_capture_exchange_initialize_solving(slice_index si,
                                                   slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  circe_instrument_solving(si,
                           interval_start,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STAnticirceCaptureExchangeDoTransfer));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void solving_instrument_anticirce_capture_exchange(void)
{
  // TODO
}
