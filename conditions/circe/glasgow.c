#include "conditions/circe/glasgow.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "solving/move_effect_journal.h"
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
void circe_glasgow_adjust_rebirth_square_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  SquareFlags const prom = context->relevant_side==White ? BlPromSq : WhPromSq;

  square const candidate = (context->rebirth_square
                            + (context->relevant_side==White
                               ? dir_up
                               : dir_down));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec(candidate),prom))
    context->rebirth_square = candidate;

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Glasgow Chess for Circe
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_glasgow_initialise_solving(slice_index si,
                                      circe_variant_type const *variant,
                                      slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (variant->determine_rebirth_square==circe_determine_rebirth_square_from_pas)
    circe_instrument_solving(si,
                             interval_start,
                             STCirceDeterminingRebirth,
                             alloc_pipe(STCirceGlasgowAdjustRebirthSquare));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
