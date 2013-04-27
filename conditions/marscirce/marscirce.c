#include "conditions/marscirce/marscirce.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <stdlib.h>

/* Generate non-capturing moves
 * @param side side for which to generate moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @param sq_real_departure real departure square of the generated moves
 */
void marscirce_generate_non_captures(Side side,
                                     piece p,
                                     square sq_generate_from,
                                     square sq_real_departure)
{
  numecoup const base = current_move[nbply];
  numecoup top_filtered = base;
  numecoup curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base);
  TraceSquare(sq_real_departure);
  TraceFunctionParamListEnd();

  gen_piece_aux(side,sq_generate_from,abs(p));

  for (curr = base+1; curr<=current_move[nbply]; ++curr)
    if (e[move_generation_stack[curr].capture]==vide)
    {
      ++top_filtered;
      move_generation_stack[top_filtered] = move_generation_stack[curr];
      move_generation_stack[top_filtered].departure = sq_real_departure;
    }

  current_move[nbply] = top_filtered;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate capturing moves
 * @param side side for which to generate moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @param sq_real_departure real departure square of the generated moves
 */
void marscirce_generate_captures(Side side,
                                 piece p,
                                 square sq_generate_from,
                                 square sq_real_departure)
{
  numecoup const base = current_move[nbply];
  numecoup top_filtered = base;
  numecoup curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base);
  TraceSquare(sq_real_departure);
  TraceFunctionParamListEnd();

  gen_piece_aux(side,sq_generate_from,abs(p));

  for (curr = base+1; curr<=current_move[nbply]; ++curr)
    if (e[move_generation_stack[curr].capture]!=vide)
    {
      ++top_filtered;
      move_generation_stack[top_filtered] = move_generation_stack[curr];
      move_generation_stack[top_filtered].departure = sq_real_departure;
    }

  current_move[nbply] = top_filtered;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param side side for which to generate moves for
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_generate_moves(Side side, piece p, square sq_departure)
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
      gen_piece_aux(side,sq_departure,abs(p));
    else
    {
      marscirce_generate_non_captures(side,p,sq_departure,sq_departure);

      if (e[sq_rebirth]==vide)
      {

        e[sq_rebirth] = e[sq_departure];
        spec[sq_rebirth] = spec[sq_departure];

        e[sq_departure] = vide;
        spec[sq_departure] = EmptySpec;

        marscirce_generate_captures(side,p,sq_rebirth,sq_departure);

        e[sq_departure]= e[sq_rebirth];
        spec[sq_departure]= spec[sq_rebirth];

        e[sq_rebirth] = vide;
        spec[sq_rebirth] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
