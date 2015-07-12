#include "conditions/circe/exchange_by_rebirth_special.h"
#include "position/pieceid.h"
#include "position/position.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "conditions/circe/circe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides slice type STCirceExchangeSpecial
 */

/* Allocate a STCirceExchangeSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_circe_exchange_special_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCirceExchangeSpecial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void circe_exchange_special_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();

  {
    move_effect_journal_index_type const rebirth = circe_find_current_rebirth();

    if (rebirth>=move_effect_journal_base[nbply]+move_effect_journal_index_offset_other_effects)
    {
      square const sq_rebirth = move_effect_journal[rebirth].u.piece_addition.added.on;
      square const sq_diagram = GetPositionInDiagram(being_solved.spec[sq_rebirth]);
      if (sq_rebirth!=initsquare
          && GetPositionInDiagram(being_solved.spec[sq_diagram])==sq_rebirth
          && TSTFLAG(being_solved.spec[sq_diagram],SLICE_STARTER(si))
          && sq_diagram!=sq_rebirth)
        pipe_solve_delegate(si);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
