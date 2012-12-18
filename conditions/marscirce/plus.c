#include "conditions/marscirce/plus.h"
#include "pydata.h"
#include "conditions/marscirce/marscirce.h"
#include "debugging/trace.h"

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

  if (from!=sq_departure && e[from]==vide)
  {
    e[from] = e[sq_departure];
    spec[from] = spec[sq_departure];

    e[sq_departure] = vide;
    spec[sq_departure] = EmptySpec;

    marscirce_generate_captures(side,p,from,sq_departure);

    e[sq_departure]= e[from];
    spec[sq_departure]= spec[from];

    e[from]= vide;
    spec[from] = EmptySpec;
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
