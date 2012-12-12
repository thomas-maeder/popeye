#include "conditions/provocateurs.h"
#include "pydata.h"
#include "debugging/trace.h"

/* Determine whether a piece is observed
 * @param sq_departure position of the piece
 * @return true iff the piece is observed
 */
boolean provocateurs_is_observed(square sq_departure)
{
  Side const side = e[sq_departure]<=roin ? Black : White;
  square const save_king_square = king_square[side];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  king_square[side] = sq_departure;
  result = rechec[side](eval_2);
  king_square[side] = save_king_square;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
