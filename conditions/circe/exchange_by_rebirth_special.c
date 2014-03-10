#include "conditions/circe/exchange_by_rebirth_special.h"
#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "conditions/circe/circe.h"
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
stip_length_type circe_exchange_special_solve(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const rebirth = circe_find_current_rebirth();

    if (rebirth>=move_effect_journal_base[nbply]+move_effect_journal_index_offset_other_effects)
    {
      square const sq_rebirth = move_effect_journal[rebirth].u.piece_addition.on;
      square const sq_diagram = GetPositionInDiagram(spec[sq_rebirth]);
      if (sq_rebirth!=initsquare
          && GetPositionInDiagram(spec[sq_diagram])==sq_rebirth
          && TSTFLAG(spec[sq_diagram],slices[si].starter)
          && sq_diagram!=sq_rebirth)
        result = solve(slices[si].next1,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
