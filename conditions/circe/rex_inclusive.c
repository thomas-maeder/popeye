#include "conditions/circe/rex_inclusive.h"
#include "pydata.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"

boolean circe_is_rex_inclusive;

/* Inialise solving in Circe rex inclusive
 * @param si identifies root slice of solving machinery
 */
void circe_rex_inclusive_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_check_validation(si,nr_sides,STValidateCheckMoveByPlayingCapture);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
