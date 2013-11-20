#include "conditions/circe/assassin.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/capture_fork.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

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
        if (is_square_observed(&validate_check))
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
stip_length_type circe_assassin_place_reborn_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square==initsquare)
    result = solve(slices[si].next1,n);
  else if (is_square_empty(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square))
  {
    move_effect_journal_do_piece_readdition(move_effect_reason_rebirth_no_choice,
                                            circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square,
                                            circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_walk,
                                            circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_spec);
    ++circe_rebirth_context_stack_pointer;
    result = solve(slices[si].next1,n);
    --circe_rebirth_context_stack_pointer;
  }
  else if (circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square==king_square[slices[si].starter])
    result = this_move_is_illegal;
  else
  {
    move_effect_journal_do_piece_removal(move_effect_reason_assassin_circe_rebirth,
                                         circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square);
    move_effect_journal_do_piece_readdition(move_effect_reason_rebirth_no_choice,
                                            circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square,
                                            circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_walk,
                                            circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].reborn_spec);
    ++circe_rebirth_context_stack_pointer;
    result = solve(slices[si].next1,n);
    --circe_rebirth_context_stack_pointer;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
  circe_instrument_solving(si,STCirceAssassinPlaceReborn);
  stip_insert_circe_capture_forks(si);

  solving_instrument_check_testing(si,STAssassinCirceCheckTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
