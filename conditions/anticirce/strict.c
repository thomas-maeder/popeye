#include "conditions/anticirce/strict.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "conditions/anticirce/anticirce.h"
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
stip_length_type anticirce_place_reborn_strict_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_removal(move_effect_reason_anticirce_rebirth,
                                       sq_arrival);

  if (is_square_empty(current_anticirce_rebirth_square[nbply]))
  {
    move_effect_journal_do_piece_readdition(move_effect_reason_anticirce_rebirth,
                                            current_anticirce_rebirth_square[nbply],
                                            anticirce_current_reborn_piece[nbply],
                                            anticirce_current_reborn_spec[nbply]);
    result = solve(slices[si].next1,n);
  }
  else
  {
    current_anticirce_rebirth_square[nbply] = initsquare;
    result = previous_move_is_illegal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_strict(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAnticircePlaceRebornStrict);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
