#include "conditions/shielded_kings.h"
#include "pydata.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <stdlib.h>

static boolean shielded_kings_test_support(square sq_observer,
                                           square sq_landing,
                                           square sq_observee)
{
  boolean result;
  Side const moving_side = e[sq_observer]>vide ? White : Black;
  Side const opponent = advers(moving_side);
  square const save_king_square = king_square[opponent];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  king_square[opponent] = sq_observer;
  result = rechec[opponent](observer_validator);
  king_square[opponent] = save_king_square;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Can a piece deliver check according to Shielded kings
 * @param sq_departure position of the piece
 * @param sq_arrival arrival square of the capture to be threatened
 * @param sq_capture typically the position of the opposite king
 */

boolean shielded_kings_validate_observation(square sq_observer,
                                            square sq_landing,
                                            square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if ((sq_observer==king_square[Black] && sq_observee==king_square[White])
      || (sq_observer==king_square[White] && sq_observee==king_square[Black]))
    /* won't work for locust Ks etc.*/
    result = !shielded_kings_test_support(sq_observee,sq_observer,sq_observer);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
