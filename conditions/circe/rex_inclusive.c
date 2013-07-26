#include "conditions/circe/rex_inclusive.h"
#include "pydata.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"

/* Validate an observation according to Circe rex. incl.
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean circe_rex_inclusive_validate_observation(slice_index si,
                                                 square sq_observer,
                                                 square sq_landing,
                                                 square sq_observee)
{
  boolean result;
  Side const side_observee = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if (sq_observee==king_square[side_observee])
  {
    square const sq_rebirth = (*circerenai)(get_walk_of_piece_on_square(sq_observee),spec[sq_observee],
                                            sq_observee,sq_observer,sq_landing,
                                            trait[nbply]);
    result = !is_square_empty(sq_rebirth) && sq_observer!=sq_rebirth;
  }
  else
    result = true;

  if (result)
    result = validate_observation_recursive(slices[si].next1,
                                            sq_observer,
                                            sq_landing,
                                            sq_observee);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 */
void circe_rex_inclusive_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_observation_testing(si,nr_sides,STTestingObservationCirceRexIncl);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
