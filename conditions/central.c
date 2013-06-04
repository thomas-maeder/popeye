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

  if (validate_observer(sq_departure,sq_arrival,sq_capture))
    result = central_can_piece_move_from(sq_departure);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_observer_supported(square sq_observer,
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

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (sq_departure==king_square[trait[nbply]])
    result = true;
  else
    result = is_square_attacked(trait[nbply],sq_departure,&central_test_supporter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Central Chess
 */
void central_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observation_validator(&is_observer_supported);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
