#include "conditions/shielded_kings.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

/* Validate an observation according to Shielded Kings
 * @return true iff the observation is valid
 */
boolean shielded_kings_validate_observation(slice_index si)
{
  square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
  boolean result;
  square const sq_observee = move_generation_stack[current_move[nbply]-1].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if ((sq_observer==king_square[Black] && sq_observee==king_square[White])
      || (sq_observer==king_square[White] && sq_observee==king_square[Black]))
  {
    /* won't work for locust Ks etc.*/
    siblingply(advers(trait[nbply]));
    current_move[nbply] = current_move[nbply-1]+1;
    move_generation_stack[current_move[nbply]-1].capture = sq_observee;
        result = !is_square_observed(&validate_observer);
    finply();
  }
  else
    result = true;

  if (result)
    result = validate_observation_recursive(slices[si].next1);

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

  stip_instrument_observation_validation(si,nr_sides,STValidatingObservationShielded);
  stip_instrument_check_validation(si,nr_sides,STValidatingObservationShielded);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
