#include "conditions/chameleon_pursuit.h"
#include "pydata.h"
#include "conditions/andernach.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type chameleon_pursuit_side_changer_solve(slice_index si,
                                                       stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_last_departure = move_generation_stack[current_move[parent_ply[nbply]]].departure;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_arrival==sq_last_departure
      && sq_departure!=prev_king_square[Black][nbply]
      && sq_departure!=prev_king_square[White][nbply])
    andernach_assume_side(advers(slices[si].starter));

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_chameleon_pursuit(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STChameleonPursuitSideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
