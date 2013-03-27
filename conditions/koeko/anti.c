#include "conditions/koeko/anti.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <stdlib.h>

nocontactfunc_t antikoeko_nocontact;


/* Validate an observation according to Patrol Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
static boolean avoid_contact_while_observing(square sq_observer,
                                             square sq_landing,
                                             square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  result = nocontact(sq_observer,sq_landing,sq_observee,antikoeko_nocontact);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type antikoeko_legality_tester_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if ((*antikoeko_nocontact)(sq_arrival))
    result = solve(slices[si].next1,n);
  else
    result = previous_move_is_illegal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Anti-Koeko
 * @param si identifies the root slice of the stipulation
 */
void antikoeko_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_instrument_moves(si,STAntiKoekoLegalityTester);

  register_observer_validator(&avoid_contact_while_observing);
  register_observation_geometry_validator(&avoid_contact_while_observing);
  register_observation_validator(&avoid_contact_while_observing);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
