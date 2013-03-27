#include "conditions/circe/rex_inclusive.h"
#include "pydata.h"
#include "debugging/trace.h"

/* Validate an observation according to Circe rex inclusive
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
static boolean avoid_observation_of_rebirthable_king(square sq_observer,
                                                     square sq_landing,
                                                     square sq_observee)
{
  boolean result;
  Side const moving = e[sq_observer]>vide ? White : Black;
  Side const opponent = advers(moving);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if (sq_observee==king_square[opponent])
  {
    square const sq_rebirth = (*circerenai)(e[sq_observee],spec[sq_observee],
                                            sq_observee,sq_observer,sq_landing,
                                            moving);
    result = e[sq_rebirth]!=vide && sq_observer!=sq_rebirth;
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Circe rex inclusive
 */
void circe_rex_inclusive_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observation_validator(&avoid_observation_of_rebirthable_king);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
