#include "conditions/circe/king_rebirth_avoider.h"
#include "conditions/circe/capture_fork.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "debugging/trace.h"

#include <assert.h>

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_king_rebirth_avoiders(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_insert_rebirth_avoider(si,STCirceKingRebirthAvoider);

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
stip_length_type circe_king_rebirth_avoider_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;
  slice_index const next = ((sq_capture==prev_king_square[White][nbply]
                             || sq_capture==prev_king_square[Black][nbply])
                            ? slices[si].next2
                            : slices[si].next1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
