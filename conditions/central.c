#include "conditions/central.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

#include "debugging/assert.h"

/* avoid stack overflow in case of cycles */
static boolean is_in_chain[maxsquare];

static boolean is_mover_supported(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceEOL();

  if (is_in_chain[sq_departure])
    result = false;
  else if (TSTFULLFLAGMASK(being_solved.spec[sq_departure],
                           BIT(trait[nbply])|BIT(Royal)))
    result = true;
  else
  {
    is_in_chain[sq_departure] = true;
    /* this is an indirectly recursive call: */
    result = is_square_observed_general(trait[nbply],sq_departure,EVALUATE(observation));
    is_in_chain[sq_departure] = false;
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
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (is_mover_supported(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void central_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_mover_supported(current_generation))
    pipe_move_generation_delegate(si);

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

  solving_instrument_move_for_piece_generation(si,nr_sides,STCentralMovesForPieceGenerator);

  stip_instrument_observation_validation(si,nr_sides,STCentralObservationValidator);
  stip_instrument_check_validation(si,nr_sides,STCentralObservationValidator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
