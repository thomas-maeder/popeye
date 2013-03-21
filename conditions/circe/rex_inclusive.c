#include "conditions/circe/rex_inclusive.h"
#include "pydata.h"
#include "debugging/trace.h"

/* Validate an observation according to Circe rex inclusive
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean circe_rex_inclusive_validate_observation(square sq_observer,
                                                 square sq_landing,
                                                 square sq_observee)
{
  boolean result;
  Side const moving = e[sq_observer]>vide ? White : Black;
  Side const opponent = advers(moving);
  square const sq_rebirth = (*circerenai)(e[king_square[opponent]],spec[king_square[opponent]],
                                          sq_observee,sq_observer,sq_landing,
                                          moving);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  result = sq_observer!=sq_rebirth;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
