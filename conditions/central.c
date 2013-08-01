#include "conditions/central.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

static boolean is_supported(square sq_departure);

static boolean validate_supporter(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (validate_observer())
    result = is_supported(sq_departure);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_supported(square sq_departure)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (sq_departure==king_square[trait[nbply]])
    result = true;
  else
  {
    move_generation_stack[current_move[nbply]-1].capture = sq_departure;
    result = is_square_observed(&validate_supporter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Central Chess
 * @return true iff the observation is valid
 */
boolean central_validate_observation(slice_index si)
{
  square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
  boolean result;
  boolean is_observer_supported;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  is_observer_supported = is_supported(sq_observer);
  finply();

  return (is_observer_supported
          && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void central_generate_moves_for_piece(slice_index si,
                                      square sq_departure,
                                      PieNam p)
{
  boolean is_mover_supported;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  is_mover_supported = is_supported(sq_departure);
  finply();

  if (is_mover_supported)
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Central Chess
 * @param si identifies root slice of solving machinery
 */
void central_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STCentralMovesForPieceGenerator);

  stip_instrument_observation_validation(si,nr_sides,STValidatingObservationCentral);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
