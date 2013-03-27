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
