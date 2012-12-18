#include "conditions/marscirce/anti.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "solving/en_passant.h"
#include "conditions/marscirce/marscirce.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param side side for which to generate moves for
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not necessarily have walk p
 */
void antimars_generate_moves(Side side, piece p, square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TracePiece(p);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  {
    square const sq_rebirth = (*marsrenai)(p,
                                           spec[sq_departure],
                                           sq_departure,initsquare,initsquare,
                                           advers(side));

    if (sq_rebirth==sq_departure)
      gen_piece_aux(side,sq_departure,p);
    else
    {
      marscirce_generate_captures(side,p,sq_departure,sq_departure);

      if (e[sq_rebirth]==vide)
      {
        spec[sq_rebirth] = spec[sq_departure];
        e[sq_rebirth] = e[sq_departure];

        e[sq_departure] = vide;
        spec[sq_departure] = EmptySpec;

        marscirce_generate_non_captures(side,p,sq_rebirth,sq_departure);

        spec[sq_departure] = spec[sq_rebirth];
        e[sq_departure] = e[sq_rebirth];

        e[sq_rebirth] = vide;
        spec[sq_rebirth]= EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void adjust(void)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  piece const pi_arriving = e[sq_arrival];

  ep[nbply] = initsquare;
  ep2[nbply] = initsquare;

  if (is_pawn(abs(pi_arriving)) && pprise[nbply]==vide)
  {
    square const sq_departure = move_generation_stack[current_move[nbply]].departure;
    square const sq_rebirth = (*marsrenai)(pi_arriving,
                                           spec[sq_arrival],
                                           sq_departure,
                                           initsquare,
                                           initsquare,
                                           advers(trait[nbply]));

    adjust_ep_squares(sq_rebirth);

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
stip_length_type antimars_en_passant_adjuster_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  adjust();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with Anti-Mars-Circe
 */
void stip_insert_antimars_en_passant_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAntiMarsCirceEnPassantAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
