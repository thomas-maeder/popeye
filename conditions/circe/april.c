#include "conditions/circe/april.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rebirth_avoider.h"
#include "pieces/pieces.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate an April Chess fork slice
 * @param is_walk_affected map a walk to a boolean value indicating which path to take
 *                         for the walk
 * @return the allocated fork
 */
static slice_index alloc_april_chess_fork(circe_variant_type const *variant)
{
  slice_index const result = alloc_fork_slice(STAprilCaptureFork,no_slice);
  SLICE_U(result).circe_handler.variant = variant;
  return result;
}

/* Instrument the solving machinery with the logic for restricting Circe to
 * certain piece walks
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_april(slice_index si,
                                    struct circe_variant_type const *variant,
                                    slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",interval_start);
  TraceFunctionParamListEnd();

  circe_insert_rebirth_avoider(si,
                               interval_start,
                               interval_start,
                               alloc_april_chess_fork(variant),
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
void april_chess_fork_solve(slice_index si)
{
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  slice_index const next = SLICE_U(si).circe_handler.variant->is_walk_affected[context->relevant_walk] ? SLICE_NEXT1(si) : SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  dispatch(next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
