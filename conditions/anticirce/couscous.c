#include "conditions/anticirce/couscous.h"
#include "conditions/circe/circe.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
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
void anticirce_couscous_make_capturee_relevant_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
    circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

    context->relevant_walk = move_effect_journal[capture].u.piece_removal.walk;
    context->relevant_spec = move_effect_journal[capture].u.piece_removal.flags;
    context->relevant_side = advers(context->relevant_side);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
