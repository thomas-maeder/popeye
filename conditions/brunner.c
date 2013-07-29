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
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean brunner_validate_observation(slice_index si,
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

  TraceValue("%u",nbply);
  TraceValue("%u\n",current_move[nbply]);
  move_generation_stack[current_move[nbply]].departure = sq_observer;
  move_generation_stack[current_move[nbply]].arrival = sq_landing;
  move_generation_stack[current_move[nbply]].capture = sq_observee;
  move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;
  result = solve(slices[temporary_hack_brunner_check_defense_finder[trait[nbply]]].next2,length_unspecified)==next_move_has_solution;
  current_move[nbply] = current_move[nbply-1]+1;

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
