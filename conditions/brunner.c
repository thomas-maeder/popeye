#include "conditions/brunner.h"
#include "solving/has_solution_type.h"
#include "solving/temporary_hacks.h"
#include "solving/check.h"
#include "solving/observation.h"
#include "solving/machinery/solve.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Validate an observation according to Brunner Chess
 * @return true iff the observation is valid
 */
boolean brunner_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (fork_solve(temporary_hack_brunner_check_defense_finder[trait[nbply]],
                       length_unspecified)
            ==next_move_has_solution);

  PUSH_OBSERVATION_TARGET_AGAIN(nbply);

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

  stip_instrument_check_validation(si,nr_sides,STBrunnerValidateCheck);
  check_no_king_is_possible();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
