#include "conditions/andernach.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Let the arriving piece of a move assume a side
 * @param side side to assume
 */
void andernach_assume_side(Side side)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  move_effect_journal_do_side_change(move_effect_reason_andernach_chess,
                                     sq_arrival,
                                     side);
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type andernach_side_changer_solve(slice_index si,
                                               stip_length_type n)
{
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]!=vide
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
void stip_insert_andernach(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAndernachSideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
