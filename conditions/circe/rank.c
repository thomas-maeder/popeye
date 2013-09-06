#include "conditions/circe/rank.h"
#include "conditions/circe/circe.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type rank_circe_determine_relevant_side_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
    square const sq_capture = move_effect_journal[capture].u.piece_removal.from;

    /* make sure that RBS are reborn on a square of the same colour as the
     * capture square */
    unsigned const row_capture = sq_capture/onerow;
    if (row_capture%2==0)
      circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_side = advers(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].relevant_side);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type rank_circe_adjust_rebirth_square_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
    square const sq_capture = move_effect_journal[capture].u.piece_removal.from;
    unsigned const row_capture = sq_capture/onerow;
    unsigned int const file_rebirth = circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square%onerow;
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = onerow*row_capture + file_rebirth;
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Override the Circe instrumentation of the solving machinery with
 * Rank Circe
 * @param si identifies root slice of stipulation
 */
void rank_circe_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STRankCirceDetermineRelevantSide);
  stip_instrument_moves(si,STRankCirceAdjustRebirthSquare);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
