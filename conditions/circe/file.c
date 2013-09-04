#include "conditions/circe/file.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/capture_fork.h"
#include "pieces/walks/classification.h"
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
stip_length_type file_circe_determine_rebirth_square_solve(slice_index si,
                                                           stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.from;
  unsigned int const col_capture = sq_capture % onerow;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_circe_relevant_side[nbply]==Black)
  {
    if (is_pawn(current_circe_relevant_piece[nbply]))
      current_circe_rebirth_square[nbply] = col_capture + (nr_of_slack_rows_below_board+1)*onerow;
    else
      current_circe_rebirth_square[nbply] = col_capture + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(current_circe_relevant_piece[nbply]))
      current_circe_rebirth_square[nbply] = col_capture + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      current_circe_rebirth_square[nbply] = col_capture + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Override the Circe instrumentation of the solving machinery with
 * File Circe
 * @param si identifies root slice of stipulation
 */
void file_circe_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  stip_instrument_moves(si,STCirceDetermineRelevantPiece);
  stip_instrument_moves(si,STFileCirceDetermineRebirthSquare);
  stip_instrument_moves(si,STCircePlaceReborn);
  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
