#include "conditions/circe/capture_fork.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rebirth_avoider.h"
#include "stipulation/fork.h"
#include "solving/binary.h"
#include "solving/fork.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Instrument the solving machinery with Circe Super (apart from the rebirth
 * square determination, whose instrumentation is elsewhere)
 * @param si identifies entry slice into solving machinery
 * @param variant identifies address of structure holding the Circe variant
 * @param interval_start type of slice that starts the sequence of slices
 *                       implementing that variant
 */
void circe_solving_instrument_nocapture_bypass(slice_index si,
                                               slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  circe_insert_rebirth_avoider(si,
                               interval_start,
                               interval_start,
                               alloc_fork_slice(STCirceCaptureFork,no_slice),
                               STCirceRebirthAvoided,
                               STCirceDoneWithRebirth);

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
void circe_capture_fork_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",context->relevant_ply);TraceEOL();

  if (context->relevant_ply==ply_nil)
    fork_solve_delegate(si);
  else
  {
    move_effect_journal_index_type const base = move_effect_journal_base[context->relevant_ply];
    move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
    move_effect_type const type = move_effect_journal[capture].type;

    TraceValue("%u",type);TraceEOL();

    binary_solve_if_then_else(si,type==move_effect_no_piece_removal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
