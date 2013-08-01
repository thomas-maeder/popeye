#include "conditions/brunner.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "solving/single_move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

/* Validate an observation according to Brunner Chess
 * @return true iff the observation is valid
 */
boolean brunner_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = solve(slices[temporary_hack_brunner_check_defense_finder[trait[nbply]]].next2,length_unspecified)==next_move_has_solution;
  current_move[nbply] = current_move[nbply-1]+1;

  if (result)
    result = validate_observation_recursive(slices[si].next1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Brunner Chess
 * @param si identifies the root slice of the solving machinery
 */
void brunner_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_check_validation(si,nr_sides,STValidatingObservationBrunner);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
