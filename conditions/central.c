#include "conditions/central.h"
#include "pydata.h"
#include "debugging/trace.h"

/* Determine whether a pice is supported
 * @param sq_departure position of the piece
 * @return true iff the piece is supported
 */
boolean central_is_supported(square sq_departure)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (sq_departure==king_square[White] || sq_departure==king_square[Black])
    result = true;
  else
  {
    piece const p = e[sq_departure];
    Side const opponent = p>=roib ? Black : White;
    square const save_king_square = king_square[opponent];
    piece const dummy = p>vide ? dummyb : dummyn;

    --nbpiece[p];
    e[sq_departure] = dummy;
    ++nbpiece[dummy];

    king_square[opponent] = sq_departure;
    result = rechec[opponent](&soutenu);
    king_square[opponent] = save_king_square;

    --nbpiece[dummy];
    e[sq_departure] = p;
    ++nbpiece[p];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
