#include "conditions/marscirce/plus.h"
#include "pydata.h"
#include "conditions/marscirce/marscirce.h"
#include "debugging/trace.h"

#include <stdlib.h>

static void generate_additional_captures_from(Side side,
                                              piece p,
                                              square from,
                                              square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TracePiece(p);
  TraceSquare(from);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (from!=sq_departure && is_square_empty(from))
  {
    occupy_square(from,abs(e[sq_departure]),spec[sq_departure]);
    empty_square(sq_departure);

    marscirce_generate_captures(side,p,from,sq_departure);

    occupy_square(sq_departure,abs(e[from]),spec[from]);
    empty_square(from);
  }

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
void plus_generate_moves(Side side, piece p, square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TracePiece(p);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  gen_piece_aux(side,sq_departure,abs(p));

  if (sq_departure==square_d4
      || sq_departure==square_e4
      || sq_departure==square_d5
      || sq_departure==square_e5)
  {
    generate_additional_captures_from(side,p,square_d4,sq_departure);
    generate_additional_captures_from(side,p,square_e4,sq_departure);
    generate_additional_captures_from(side,p,square_d5,sq_departure);
    generate_additional_captures_from(side,p,square_e5,sq_departure);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a specific side is in check in Echecs Plus
 * @param side the side
 * @param evaluate filter for king capturing moves
 * @return true iff side is in check
 */
boolean plusechecc(Side side, evalfunction_t *evaluate)
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
          && pos_checking!=king_square[side]) /* exclude nK */
      {
        if (pos_checking==square_d4 || pos_checking==square_d5 || pos_checking==square_e4 || pos_checking==square_e5)
          result = (mars_does_piece_deliver_check(side,pos_checking,square_d4)
                    || mars_does_piece_deliver_check(side,pos_checking,square_d5)
                    || mars_does_piece_deliver_check(side,pos_checking,square_e4)
                    || mars_does_piece_deliver_check(side,pos_checking,square_e5));
        else
          result = mars_does_piece_deliver_check(side,pos_checking,pos_checking);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
