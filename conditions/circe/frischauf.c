#include "conditions/circe/frischauf.h"
#include "conditions/conditions.h"
#include "conditions/circe/circe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "position/position.h"
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
void frischauf_adjust_rebirth_square_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFLAG(context->relevant_spec,FrischAuf))
  {
    unsigned int col = context->relevant_square % onerow;
    unsigned int const row = (context->relevant_side==Black
                              ? nr_of_slack_rows_below_board+nr_rows_on_board-1
                              : nr_of_slack_rows_below_board);

    context->rebirth_square = col + onerow*row;
  }

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static void mark_promotees(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[parent_ply[nbply]+1]<=top);

  for (curr = move_effect_journal_base[parent_ply[nbply]+1]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_walk_change
        && move_effect_journal[curr].reason==move_effect_reason_pawn_promotion)
    {
      square const on = move_effect_journal[curr].u.piece_change.on;
      Flags flags = being_solved.spec[on];
      SETFLAG(flags,FrischAuf);
      move_effect_journal_do_flags_change(move_effect_reason_frischauf_mark_promotee,
                                          on,flags);
    }

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
void frischauf_promotee_marker_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  mark_promotees();
  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with promotee markers
 * @param si root slice of solving machinery
 */
void frischauf_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STFrischaufPromoteeMarker);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
