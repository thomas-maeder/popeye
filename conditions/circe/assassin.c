#include "conditions/circe/assassin.h"
#include "conditions/circe/circe.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

circe_assassin_use_whom_type circe_assassin_use_whom;

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
  Flags side_mask = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_attacked,"");
  TraceFunctionParamListEnd();

  if (circe_assassin_use_whom&circe_assassin_use_attacked)
    side_mask |= BIT(side_attacked);
  if (circe_assassin_use_whom&circe_assassin_use_attacking)
    side_mask |= BIT(advers(side_attacked));

  for (bnp = boardnum; *bnp; ++bnp)
    if (spec[*bnp]&side_mask)
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
 */
void circe_assassin_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  instrument(si);
  circe_assassin_use_whom |= circe_assassin_use_attacked;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the solving machinery with Assassin Anticirce
 * @param si identifies root slice of stipulation
 */
void anticirce_assassin_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  instrument(si);
  circe_assassin_use_whom |= circe_assassin_use_attacking;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
