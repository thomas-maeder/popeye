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
    square const sq_rebirth = (*marsrenai)(abs(p),
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
        occupy_square(sq_rebirth,abs(e[sq_departure]),spec[sq_departure]);
        empty_square(sq_departure);

        marscirce_generate_captures(side,p,sq_rebirth,sq_departure);

        occupy_square(sq_departure,abs(e[sq_rebirth]),spec[sq_rebirth]);
        empty_square(sq_rebirth);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a specific piece delivers check to a specific side from a
 * specific rebirth square
 * @param side potentially in check
 * @param pos_checking potentially delivering check ...
 * @param sq_rebrirth ... from this square
 * @note the piece on pos_checking must belong to advers(side)
 */
boolean mars_does_piece_deliver_check(Side side, square pos_checking, square sq_rebirth)
{
  boolean result = false;
  PieNam const pi_checking = abs(e[pos_checking]);
  Flags const spec_checking = spec[pos_checking];

  if (e[sq_rebirth]==vide || sq_rebirth==pos_checking)
  {
    empty_square(pos_checking);
    occupy_square(sq_rebirth,pi_checking,spec_checking);

    nextply();
    trait[nbply] = advers(side);
    fromspecificsquare = sq_rebirth;
    result = (*checkfunctions[pi_checking])(king_square[side],pi_checking,&eval_fromspecificsquare);
    finply();

    empty_square(sq_rebirth);
    occupy_square(pos_checking,pi_checking,spec_checking);
  }

  return result;
}

/* Determine whether a specific side is in check in Mars Circe
 * @param side the side
 * @param evaluate filter for king capturing moves
 * @return true iff side is in check
 */
boolean marsechecc(Side side, evalfunction_t *evaluate)
{
  int i,j;
  square square_h = square_h8;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  for (i= nr_rows_on_board; i>0 && !result; i--, square_h += dir_down)
  {
    square pos_checking = square_h;
    for (j= nr_files_on_board; j>0 && !result; j--, pos_checking += dir_left)
      if (piece_belongs_to_opponent(pos_checking,side)
          && pos_checking!=king_square[side]   /* exclude nK */)
      {
        PieNam const pi_checking = abs(e[pos_checking]);
        Flags const spec_checking = spec[pos_checking];
        square const sq_rebirth = (*marsrenai)(pi_checking,spec_checking,pos_checking,initsquare,initsquare,side);
        result = mars_does_piece_deliver_check(side,pos_checking,sq_rebirth);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
