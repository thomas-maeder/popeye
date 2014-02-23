#include "conditions/circe/assassin.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/rebirth_avoider.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean assassin_circe_check_tester_is_in_check(slice_index si, Side side_in_check)
{
  boolean assassinable = false;
  Side const side_checking = advers(side_in_check);

  if (is_in_check_recursive(slices[si].next1,side_in_check))
    assassinable = true;
  else
  {
    square const *bnp;

    siblingply(side_checking);
    push_observation_target(initsquare);

    for (bnp = boardnum; *bnp; bnp++)
    {
      PieNam const p = get_walk_of_piece_on_square(*bnp);

      if (p!=Empty
          && p!=King && TSTFLAG(spec[*bnp],side_in_check)
          && rennormal(p,spec[*bnp],*bnp,side_checking)==king_square[side_in_check])
      {
        replace_observation_target(*bnp);
        if (is_square_observed(EVALUATE(check)))
        {
          assassinable = true;
          break;
        }
      }
    }

    finply();
  }

  return assassinable;
}

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
stip_length_type circe_assassin_assassinate_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  square const sq_rebirth = context->rebirth_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_rebirth==king_square[slices[si].starter])
    result = this_move_is_illegal;
  else
  {
    move_effect_journal_do_piece_removal(move_effect_reason_assassin_circe_rebirth,
                                         sq_rebirth);
    result = solve(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void append_assassin(slice_index si, stip_structure_traversal*st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype =  alloc_pipe(STCirceAssassinAssassinate);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void assassin_circe_initalise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  circe_instrument_solving(si,STCirceDetermineRebirthSquare);
  circe_instrument_solving(si,STCircePlacingReborn);
  circe_instrument_solving(si,STCircePlaceReborn);
  stip_insert_rebirth_avoider(si,STCirceTestRebirthSquareEmpty,STCirceRebirthOnNonEmptySquare,STCircePlacingReborn);
  stip_insert_circe_capture_forks(si);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STCirceRebirthOnNonEmptySquare,
                                             &append_assassin);
    stip_traverse_structure(si,&st);
  }

  solving_instrument_check_testing(si,STAssassinCirceCheckTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
