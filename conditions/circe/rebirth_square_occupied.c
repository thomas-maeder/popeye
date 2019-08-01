#include "conditions/circe/rebirth_square_occupied.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/assassin.h"
#include "conditions/circe/parachute.h"
#include "position/position.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "solving/binary.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

/* Retrieve the behaviour of a Circe variant if the rebirth square is occupied
 * @param variant address of the structure holding the variant
 * @return the enumerator identifying the behaviour
 */
circe_behaviour_on_occupied_rebirth_square_type
circe_get_on_occupied_rebirth_square(circe_variant_type const *variant)
{
  if (variant->on_occupied_rebirth_square==circe_on_occupied_rebirth_square_default)
    return variant->on_occupied_rebirth_square_default;
  else
    return variant->on_occupied_rebirth_square;
}

/* Cause moves with Circe rebirth on an occupied square to be played without
 * rebirth
 * @param si entry slice into the solving machinery
 * @param interval_start start of the slices interval where to instrument
 */
static void no_rebirth_on_occupied_square(slice_index si,
                                          slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  circe_insert_rebirth_avoider(si,
                               interval_start,
                               STCirceDeterminedRebirth,
                               alloc_fork_slice(STCirceTestRebirthSquareEmpty,no_slice),
                               STCirceRebirthOnNonEmptySquare,
                               STCirceDoneWithRebirth);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Cause moves with Circe rebirth on an occupied square to not be played
 * @param si entry slice into the solving machinery
 * @param interval_start start of the slices interval where to instrument
 */
static void stop_rebirth_on_occupied_square(slice_index si,
                                            slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  circe_insert_rebirth_avoider(si,
                               interval_start,
                               STCirceDeterminedRebirth,
                               alloc_fork_slice(STCirceTestRebirthSquareEmpty,no_slice),
                               STCirceRebirthOnNonEmptySquare,
                               STCirceDoneWithRebirth);

  circe_instrument_solving(si,
                           interval_start,
                           STCirceRebirthOnNonEmptySquare,
                           alloc_pipe(STSupercircePreventRebirthOnNonEmptySquare));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Deal with the situation where a rebirth is to occur on an occupied square
 * @param si identifies entry slice into solving machinery
 * @param variant identifies address of structure holding the Circe variant
 * @param interval_start type of slice that starts the sequence of slices
 *                       implementing that variant
 */
void circe_solving_instrument_rebirth_on_occupied_square(slice_index si,
                                                         circe_variant_type const *variant,
                                                         slice_type interval_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_type,interval_start);
  TraceFunctionParamListEnd();

  switch (circe_get_on_occupied_rebirth_square(variant))
  {
    case circe_on_occupied_rebirth_square_assassinate:
      circe_assassin_initialise_solving(si,interval_start);
      break;

    case circe_on_occupied_rebirth_square_parachute:
      circe_parachute_initialise_solving(si,interval_start);
      break;

    case circe_on_occupied_rebirth_square_volcanic:
      circe_volcanic_initialise_solving(si,interval_start);
      break;

    case circe_on_occupied_rebirth_square_strict:
      stop_rebirth_on_occupied_square(si,interval_start);
      break;

    case circe_on_occupied_rebirth_square_relaxed:
      if (variant->rebirth_reason==move_effect_reason_rebirth_choice)
        stop_rebirth_on_occupied_square(si,interval_start);
      else
        no_rebirth_on_occupied_square(si,interval_start);
      break;

    default:
      assert(0);
      break;
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
void circe_test_rebirth_square_empty_solve(slice_index si)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(context->reborn_walk!=Empty);

  TraceSquare(context->rebirth_square);
  TraceValue("%u",is_square_empty(context->rebirth_square));
  TraceEOL();

  binary_solve_if_then_else(si,!is_square_empty(context->rebirth_square));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
