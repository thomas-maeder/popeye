#include "solving/en_passant.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

square ep[maxply+1];
square ep2[maxply+1];

/* Adjust en passant possibilities of the following move after a non-capturing
 * move
 * @param sq_multistep_departure departure square of pawn move
 */
void adjust_ep_squares(square sq_multistep_departure)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  piece const pi_moving = trait[nbply]==White ? abs(pjoue[nbply]) : -abs(pjoue[nbply]);
  switch (pi_moving)
  {
    case pb:
    case reversepn:
      switch (sq_arrival-sq_multistep_departure)
      {
        case 2*dir_up: /* ordinary or Einstein double step */
          ep[nbply] = (sq_multistep_departure+sq_arrival) / 2;
          break;
        case 3*dir_up: /* Einstein triple step */
          ep[nbply] = (sq_multistep_departure+sq_arrival+sq_arrival) / 3;
          ep2[nbply] = (sq_multistep_departure+sq_multistep_departure+sq_arrival) / 3;
          break;
        default:
          break;
      }
      break;

    case pn:
    case reversepb:
      switch (sq_arrival-sq_multistep_departure)
      {
        case 2*dir_down: /* ordinary or Einstein double step */
          ep[nbply] = (sq_multistep_departure+sq_arrival) / 2;
          break;
        case 3*dir_down: /* Einstein triple step */
          ep[nbply] = (sq_multistep_departure+sq_arrival+sq_arrival) / 3;
          ep2[nbply] = (sq_multistep_departure+sq_multistep_departure+sq_arrival) / 3;
          break;
        default:
          break;
      }
      break;

    case pbb:
      if (sq_arrival-sq_multistep_departure>=2*dir_up-2)
        ep[nbply] = (sq_multistep_departure+sq_arrival) / 2;
      break;

    case pbn:
      if (sq_arrival-sq_multistep_departure<=2*dir_down+2)
        ep[nbply] = (sq_multistep_departure+sq_arrival) / 2;
      break;

    default:
      break;
  }
}

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
stip_length_type en_passant_adjuster_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  ep[nbply] = initsquare;
  ep2[nbply] = initsquare;

  if (is_pawn(abs(pjoue[nbply])) && pprise[nbply]==vide)
    adjust_ep_squares(move_generation_stack[current_move[nbply]].departure);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_en_passant_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STEnPassantAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
