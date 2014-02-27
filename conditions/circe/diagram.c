#include "conditions/circe/diagram.h"
#include "conditions/circe/circe.h"
#include "position/pieceid.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

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
stip_length_type diagram_circe_determine_rebirth_square_solve(slice_index si,
                                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = GetPositionInDiagram(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_spec);
  circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_reason = move_effect_reason_rebirth_no_choice;
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
