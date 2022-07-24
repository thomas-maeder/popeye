#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/rebirth_avoider.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "stipulation/fork.h"
#include "solving/binary.h"
#include "debugging/trace.h"

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
void circe_prevent_king_rebirth_solve(slice_index si)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",TSTFLAG(context->reborn_spec,Royal));TraceEOL();

  binary_solve_if_then_else(si,TSTFLAG(context->reborn_spec,Royal));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise solving in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start slice type starting the sequence of slices for the variant
 */
void circe_rex_inclusive_initialise_solving(slice_index si,
                                            circe_variant_type const *variant,
                                            slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!variant->is_rex_inclusive)
    circe_insert_rebirth_avoider(si,
                                 interval_start,
                                 interval_start,
                                 alloc_fork_slice(STCircePreventKingRebirth,no_slice),
                                 STCirceRebirthAvoided,
                                 STCirceDoneWithRebirth);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise check validation in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 */
void circe_rex_inclusive_initialise_check_validation(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_check_validation(si,nr_sides,STValidateCheckMoveByPlayingCapture);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
