#include "conditions/lortap.h"
#include "pydata.h"
#include "debugging/trace.h"

/* Determine whether a pice is supported, disabling it from capturing
 * @param sq_departure position of the piece
 * @return true iff the piece is supported
 */
boolean lortap_is_supported(square sq_departure)
{
  Side const opponent = e[sq_departure]>=roib ? Black : White;
  square const save_king_square = king_square[opponent];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  king_square[opponent] = sq_departure;
  result = rechec[opponent](eval_ortho);
  king_square[opponent] = save_king_square;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
