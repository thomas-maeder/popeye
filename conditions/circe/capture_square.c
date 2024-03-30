#include "conditions/circe/capture_square.h"
#include "conditions/circe/circe.h"
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
void circe_determine_rebirth_square_capture_square_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_ply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_ply!=ply_nil);

  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = sq_capture;

  TraceSquare(sq_capture);
  TraceSquare(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square);
  TraceEOL();

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
