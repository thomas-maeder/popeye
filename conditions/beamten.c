#include "conditions/beamten.h"
#include "pydata.h"
#include "solving/observation.h"
#include "debugging/trace.h"

/* Determine whether a Beamter piece is observed
 * @param sq_departure position of the piece
 * @return true iff the piece is observed, enabling it to move
 */
boolean beamten_is_observed(square sq_departure)
{
  Side const side = e[sq_departure]<=roin ? Black : White;
  square const save_king_square = king_square[side];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  king_square[side] = sq_departure;
  result = rechec[side](observer_validator);
  king_square[side] = save_king_square;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Beamten Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean beamten_validate_observation(square sq_observer,
                                     square sq_landing,
                                     square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if (CondFlag[beamten] || TSTFLAG(spec[sq_observer],Beamtet))
    result = beamten_is_observed(sq_observer);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
