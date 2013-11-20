#include "conditions/circe/capture_fork.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/rex_inclusive.h"
#include "pieces/pieces.h"
#include "stipulation/has_solution_type.h"
#include "solving/move_effect_journal.h"
#include "conditions/circe/circe.h"
#include "debugging/trace.h"

#include <assert.h>

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
stip_length_type circe_capture_fork_solve(slice_index si,
                                           stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  Flags const removedspec = move_effect_journal[capture].u.piece_removal.removedspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_no_piece_removal)
    result = solve(slices[si].next2,n);
  else if (circe_is_rex_inclusive || !TSTFLAG(removedspec,Royal))
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_reason = move_effect_reason_rebirth_no_choice;
    result = solve(slices[si].next1,n);
  }
  else
    result = solve(slices[si].next2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_capture_forks(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_insert_rebirth_avoider(si,STCirceCaptureFork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
