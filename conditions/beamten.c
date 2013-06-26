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
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();


  nextply();
  trait[nbply] = advers(trait[parent_ply[nbply]]);

  result = is_square_attacked(sq_departure,&validate_observer);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean avoid_unobserved_observation(square sq_observer,
                                            square sq_landing,
                                            square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  result = !TSTFLAG(spec[sq_observer],Beamtet) || beamten_is_observed(sq_observer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Beamten Chess
 */
void beamten_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observation_validator(&avoid_unobserved_observation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
