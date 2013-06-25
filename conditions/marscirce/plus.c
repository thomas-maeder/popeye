#include "conditions/marscirce/plus.h"
#include "pydata.h"
#include "conditions/marscirce/marscirce.h"
#include "debugging/trace.h"

static void generate_additional_captures_from(Side side,
                                              PieNam p,
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
    occupy_square(from,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
    empty_square(sq_departure);

    marscirce_generate_captures(side,p,from,sq_departure);

    occupy_square(sq_departure,get_walk_of_piece_on_square(from),spec[from]);
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
void plus_generate_moves(Side side, PieNam p, square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TracePiece(p);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  gen_piece_aux(side,sq_departure,p);

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

/* Determine whether a side observes a specific square
 * @param side_observing the side
 * @param sq_target square potentially observed
 * @return true iff side is in check
 */
boolean plus_is_square_observed(Side side_observing,
                                square sq_target,
                                evalfunction_t *evaluate)
{
  int i,j;
  Side const side_observed = advers(side_observing);
  square square_h = square_h8;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_observing,"");
  TraceFunctionParamListEnd();

  for (i= nr_rows_on_board; i>0 && !result; i--, square_h += dir_down)
  {
    square pos_checking = square_h;
    for (j= nr_files_on_board; j>0 && !result; j--, pos_checking += dir_left)
      if (piece_belongs_to_opponent(pos_checking,side_observed)
          && pos_checking!=sq_target) /* exclude nK */
      {
        if (pos_checking==square_d4 || pos_checking==square_d5 || pos_checking==square_e4 || pos_checking==square_e5)
          result = (mars_is_square_observed_by(side_observing,pos_checking,square_d4,sq_target,evaluate)
                    || mars_is_square_observed_by(side_observing,pos_checking,square_d5,sq_target,evaluate)
                    || mars_is_square_observed_by(side_observing,pos_checking,square_e4,sq_target,evaluate)
                    || mars_is_square_observed_by(side_observing,pos_checking,square_e5,sq_target,evaluate));
        else
          result = mars_is_square_observed_by(side_observing,pos_checking,pos_checking,sq_target,evaluate);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
