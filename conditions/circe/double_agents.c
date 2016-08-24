#include "conditions/circe/double_agents.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "pieces/attributes/neutral/neutral.h"
#include "position/position.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Instrument the solving machinery with Circe Turncoats
 * @param si identifies entry slice into solving machinery
 * @param variant identifies address of structure holding the Circe variant
 * @param interval_start type of slice that starts the sequence of slices
 *                       implementing that variant
 */
void circe_solving_instrument_turncoats(slice_index si,
                                        circe_variant_type const *variant,
                                        slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  circe_instrument_solving(si,
                           interval_start,
                           STCirceDeterminingRebirth,
                           alloc_pipe(STCirceDoubleAgentsAdaptRebornSide));

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
void circe_doubleagents_adapt_reborn_side_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const rebirth = circe_find_current_rebirth();
    if (rebirth>=move_effect_journal_base[nbply]+move_effect_journal_index_offset_other_effects
        && !is_piece_neutral(move_effect_journal[rebirth].u.piece_addition.added.flags))
      move_effect_journal_do_side_change(move_effect_reason_circe_turncoats,
                                         move_effect_journal[rebirth].u.piece_addition.added.on);
  }

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
