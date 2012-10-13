#include "conditions/phantom.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/en_passant.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static void adjust(Side trait_ply)
{
  numecoup const coup_id = current_move[nbply];
  square const sq_arrival = move_generation_stack[coup_id].arrival;

  ep[nbply] = initsquare;
  ep2[nbply] = initsquare;

  if (is_pawn(abs(e[sq_arrival])) && pprise[nbply]==vide)
  {
    square const sq_departure = move_generation_stack[coup_id].departure;
    square const sq_multistep_departure = rennormal(pjoue[nbply],
                                                    spec[sq_arrival],
                                                    sq_departure,
                                                    initsquare,
                                                    initsquare,
                                                    advers(trait_ply));
    adjust_ep_squares(sq_multistep_departure);

    /* In positions like pieces black pe6 white pd5, we can't tell whether
     * the black pawn's move to e5 is a double step.
     * As the ep capturer normally has the burden of proof, let's disallow
     * ep capture in that case for now.
     */
    if (ep[nbply]==sq_departure)
      ep[nbply] = initsquare;
    if (ep2[nbply]==sq_departure)
      ep2[nbply] = initsquare;
  }
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type phantom_en_passant_adjuster_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  adjust(slices[si].starter);
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_phantom_en_passant_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STPhantomChessEnPassantAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
