#include "conditions/circe/assassin.h"
#include "position/position.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rebirth_avoider.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "position/piece_removal.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
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
void circe_assassin_assassinate_solve(slice_index si)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  square const sq_rebirth = context->rebirth_square;
  Flags const mask = BIT(SLICE_STARTER(si))|BIT(Royal);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFULLFLAGMASK(being_solved.spec[sq_rebirth],mask))
    solve_result = this_move_is_illegal;
  else
  {
    move_effect_journal_do_piece_removal(move_effect_reason_assassin_circe_rebirth,
                                         sq_rebirth);
    pipe_dispatch_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean circe_assassin_all_piece_observation_tester_is_in_check(slice_index si,
                                                                Side side_attacked)
{
  boolean result = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_attacked);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; ++bnp)
    if (TSTFLAG(being_solved.spec[*bnp],side_attacked))
    {
      replace_observation_target(*bnp);
      if (is_square_observed(EVALUATE(check)))
      {
        result = true;
        break;
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* test all attacked pieces, not just the king */
static void substitute_all_pieces_observation_tester(slice_index si, stip_structure_traversal*st)
{
  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(STCirceAssassinAllPieceObservationTester));
}

static void instrument(slice_index si)
{
  /* we have to actually play potentially assassinating moves
   */
  stip_instrument_check_validation(si,
                                   nr_sides,
                                   STValidateCheckMoveByPlayingCapture);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STKingSquareObservationTester,
                                             &substitute_all_pieces_observation_tester);
    stip_traverse_structure(si,&st);
  }
}

/* Initialise the solving machinery with Assassin Circe
 * @param si identifies root slice of stipulation
 * @param interval_start start of the slices interval to be initialised
 */
void circe_assassin_initialise_solving(slice_index si,
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
                               STCircePlacingReborn);
  circe_instrument_solving(si,
                           interval_start,
                           STCirceRebirthOnNonEmptySquare,
                           alloc_pipe(STCirceAssassinAssassinate));

  instrument(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
