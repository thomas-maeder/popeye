#include "conditions/central.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

static boolean is_mover_supported_recursive(square sq_departure);

static boolean validate_supporter(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = (validate_observer()
            && is_mover_supported_recursive(move_generation_stack[current_move[nbply]-1].departure));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_mover_supported_recursive(square sq_departure)
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

static boolean is_mover_supported(numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  result = is_mover_supported_recursive(move_generation_stack[n].departure);
  finply();

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
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (is_mover_supported(current_move[nbply]-1)
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void central_generate_moves_for_piece(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (is_mover_supported(current_generation))
    generate_moves_for_piece(slices[si].next1,p);

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
