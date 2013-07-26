#include "conditions/shielded_kings.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

/* Validate an observation according to Shielded Kings
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean shielded_kings_validate_observation(slice_index si,
                                            square sq_observer,
                                            square sq_landing,
                                            square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if ((sq_observer==king_square[Black] && sq_observee==king_square[White])
      || (sq_observer==king_square[White] && sq_observee==king_square[Black]))
  {
    /* won't work for locust Ks etc.*/
    nextply(advers(trait[nbply]));
    result = !is_square_observed(sq_observee,&validate_observer);
    finply();
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

/* Inialise solving in Shielded kings
 * @param si identifies the root slice of the solving machinery
 */
void shielded_kings_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_observation_testing(si,nr_sides,STTestingObservationShielded);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
