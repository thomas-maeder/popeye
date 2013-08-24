#include "conditions/circe/frischauf.h"
#include "pieces/pieces.h"
#include "conditions/andernach.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

static void mark_promotees(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[parent_ply[nbply]+1]<=top);

  for (curr = move_effect_journal_base[parent_ply[nbply]+1]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_change
        && move_effect_journal[curr].reason==move_effect_reason_pawn_promotion)
    {
      square const on = move_effect_journal[curr].u.piece_change.on;
      Flags flags = spec[on];
      SETFLAG(flags,FrischAuf);
      move_effect_journal_do_flags_change(move_effect_reason_frischauf_mark_promotee,
                                          on,flags);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type frischauf_promotee_marker_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  mark_promotees();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_frischauf_promotee_markers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STFrischaufPromoteeMarker);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
