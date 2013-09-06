#include "conditions/circe/turncoats.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "conditions/circe/circe.h"
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
stip_length_type circe_turncoats_side_changer_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const rebirth = circe_find_current_rebirth();

    if (rebirth!=move_effect_journal_base[nbply+1])
    {
      square const sq_rebirth = move_effect_journal[rebirth].u.piece_addition.on;

      if (!TSTFLAG(spec[sq_rebirth],trait[nbply]))
        move_effect_journal_do_side_change(move_effect_reason_circe_turncoats,
                                           sq_rebirth);
    }
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Override the Circe instrumentation of the solving machinery with
 * Circe Turncoats
 * @param si identifies root slice of stipulation
 */
void circe_turncoats_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceTraitorSideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
