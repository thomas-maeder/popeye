#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rebirth_avoider.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

boolean circe_is_rex_inclusive;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_prevent_king_rebirth_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (TSTFLAG(context->relevant_spec,Royal))
    result = solve(slices[si].next2,n);
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 */
void circe_rex_inclusive_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!circe_is_rex_inclusive)
    circe_insert_rebirth_avoider(si,
                                 STMove,
                                 STCircePreventKingRebirth,
                                 STCirceRebirthAvoided,
                                 STLandingAfterCirceRebirthHandler);

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

  if (circe_is_rex_inclusive)
    stip_instrument_check_validation(si,nr_sides,STValidateCheckMoveByPlayingCapture);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
