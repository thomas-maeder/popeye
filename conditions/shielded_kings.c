#include "conditions/shielded_kings.h"
#include "pydata.h"
#include "solving/observation.h"
#include "debugging/trace.h"

static boolean avoid_observation_of_shielded(square sq_observer,
                                             square sq_landing,
                                             square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  if ((sq_observer==king_square[Black] && sq_observee==king_square[White])
      || (sq_observer==king_square[White] && sq_observee==king_square[Black]))
  {
    /* won't work for locust Ks etc.*/
    nextply(advers(trait[nbply]));
    result = !is_square_attacked(sq_observee,&validate_observer);
    finply();
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Shielded kings
 */
void shielded_kings_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observation_validator(&avoid_observation_of_shielded);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
