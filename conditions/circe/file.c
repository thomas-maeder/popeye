#include "conditions/circe/file.h"
#include "conditions/circe/circe.h"
#include "pieces/walks/classification.h"
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
void file_circe_determine_rebirth_square_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
  unsigned int const col_capture = sq_capture % onerow;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_side==Black)
  {
    if (is_pawn(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_walk))
      circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = col_capture + (nr_of_slack_rows_below_board+1)*onerow;
    else
      circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = col_capture + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_walk))
      circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = col_capture + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = col_capture + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
