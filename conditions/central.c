#include "conditions/central.h"
#include "pydata.h"
#include "solving/observation.h"
#include "debugging/trace.h"

static boolean central_test_supporter(square sq_departure,
                                      square sq_arrival,
                                      square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  if ((*observer_validator)(sq_departure,sq_arrival,sq_capture))
    result = central_can_piece_move_from(sq_departure);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Central Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean central_validate_observation(square sq_observer,
                                     square sq_landing,
                                     square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  result = central_can_piece_move_from(sq_observer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a piece can legally move according to Central Chess
 * @param sq_departure departure square
 * @return true iff the piece can legally move
 */
boolean central_can_piece_move_from(square sq_departure)
{
  boolean result;
  Side const moving = e[sq_departure]>vide ? White : Black;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (sq_departure==king_square[moving])
    result = true;
  else
  {
    Side const opponent = advers(moving);
    square const save_opponent_king_square = king_square[opponent];

    king_square[opponent] = sq_departure;
    result = rechec[opponent](&central_test_supporter);
    king_square[opponent] = save_opponent_king_square;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
